import argparse
import os
import json
import time
from datetime import datetime
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.tensorboard import SummaryWriter
from tqdm import tqdm
import numpy as np
from dataloader import get_dataloader, AVDataset
from models import CNN, MLP, Transformer
import ray
import pandas as pd
from ray import tune
from ray import train as ray_train

def compute_dataset_stats(dataloader):
    """
    Compute mean and std of the dataset.
    Assumes dataloader returns images in (B, T, C, H, W) with values 0-1.
    """
    print("Computing dataset statistics...")
    
    # Initialize variables
    # We want mean/std per channel (C)
    # We'll need to know C first. We can get it from the first batch.
    
    mean = None
    var = None
    total_pixels = 0
    
    # We use a streaming algorithm (Welford's) or simple two-pass.
    # Given we might have memory constraints, two-pass is safer than storing all, 
    # but iterating twice is slow.
    # Simple accumulation of sum(x) and sum(x^2) is prone to numerical instability but 
    # for 0-1 images and float32 it's usually acceptable if dataset isn't massive.
    
    channel_sum = None
    channel_sq_sum = None
    
    # Action stats
    action_sum = None
    action_sq_sum = None
    total_actions = 0
    
    for batch in tqdm(dataloader, desc="Computing stats"):
        images = batch['image'] # (B, T, C, H, W)
        actions = batch['action'] # (B, T, 2)
        
        B, T, C, H, W = images.shape
        
        if channel_sum is None:
            channel_sum = torch.zeros(C, dtype=torch.float64)
            channel_sq_sum = torch.zeros(C, dtype=torch.float64)
            
        # Image stats
        # Flatten: (B, T, C, H, W) -> (B*T*H*W, C)
        # Permute to (B, T, H, W, C) first
        pixels = images.permute(0, 1, 3, 4, 2).reshape(-1, C)
        
        channel_sum += pixels.sum(dim=0).double()
        channel_sq_sum += (pixels ** 2).sum(dim=0).double()
        total_pixels += pixels.shape[0]
        
        # Action stats
        if action_sum is None:
            action_sum = torch.zeros(2, dtype=torch.float64)
            action_sq_sum = torch.zeros(2, dtype=torch.float64)
            
        # Flatten actions: (B, T, 2) -> (B*T, 2)
        acts = actions.reshape(-1, 2)
        action_sum += acts.sum(dim=0).double()
        action_sq_sum += (acts ** 2).sum(dim=0).double()
        total_actions += acts.shape[0]
        
    # Image Mean/Std
    mean = channel_sum / total_pixels
    var = (channel_sq_sum / total_pixels) - (mean ** 2)
    std = torch.sqrt(var)
    
    # Action Mean/Std
    act_mean = action_sum / total_actions
    act_var = (action_sq_sum / total_actions) - (act_mean ** 2)
    act_std = torch.sqrt(act_var)
    
    # Convert to float32 list
    mean_list = mean.float().tolist()
    std_list = std.float().tolist()
    
    act_mean_list = act_mean.float().tolist()
    act_std_list = act_std.float().tolist()
    
    print(f"Dataset Stats - Image Mean: {mean_list}, Std: {std_list}")
    print(f"Dataset Stats - Action Mean: {act_mean_list}, Std: {act_std_list}")
    
    return mean_list, std_list, act_mean_list, act_std_list



def train(args):
    # Setup paths
    dataset_dir = os.path.expanduser('~/roboracer_ws/data/rosbags_processed/datasets')
    metadata_file = os.path.join(dataset_dir, f"{args.dataset}.json")
    
    if not os.path.exists(metadata_file):
        raise FileNotFoundError(f"Dataset metadata file not found: {metadata_file}")
        
    # Create experiment directory
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    experiment_name = f"{timestamp}_{args.model}_{args.dataset}"
    experiment_dir = os.path.expanduser(f"~/roboracer_ws/data/experiments/{experiment_name}")
    models_dir = os.path.join(experiment_dir, "models")
    log_dir = os.path.join(experiment_dir, "tensorboard")
    
    os.makedirs(models_dir, exist_ok=True)
    os.makedirs(log_dir, exist_ok=True)
    
    print(f"Starting experiment: {experiment_name}")
    
    # Device
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"Using device: {device}")
    
    # Load Dataset to get dimensions
    print("Loading dataset metadata...")
    
    # 1. Estimate Data Loader Memory Usage
    # Create a temporary loader to fetch one sample
    # Pass dataset_data if available
    dataset_data = getattr(args, 'dataset_data', None)
    
    temp_loader = get_dataloader(metadata_file, split='train', batch_size=1, shuffle=False, 
                                num_workers=0, pin_memory=False, prefetch_factor=None, persistent_workers=False,
                                dataset_data=dataset_data)
    
    print("Estimating memory usage...")
    sample_batch = next(iter(temp_loader))
    
    # Calculate memory per sample (in bytes)
    # image: (B, T, C, H, W) -> float32 (4 bytes) typically
    # action: (B, T, 2)
    img_mem = sample_batch['image'].element_size() * sample_batch['image'].nelement()
    act_mem = sample_batch['action'].element_size() * sample_batch['action'].nelement()
    # Add overhead safety factor (Python objects, internal buffers) - conservatively 1.5x
    sample_mem_est = (img_mem + act_mem) * 1.5
    
    batch_mem_est = sample_mem_est * args.batch_size
    print(f"Estimated memory per sample: {sample_mem_est / 1024**2:.2f} MB")
    print(f"Estimated memory per batch (batch_size={args.batch_size}): {batch_mem_est / 1024**2:.2f} MB")
    
    # Target Memory Limit for Data Loading Buffers (e.g. 90GB)
    MEMORY_LIMIT_GB = 90
    MEMORY_LIMIT_BYTES = MEMORY_LIMIT_GB * 1024**3
    
    # Total Buffer Memory ~= (num_workers * prefetch_factor * batch_size_memory)
    # We want Total Buffer Memory < MEMORY_LIMIT_BYTES
    
    # Solve for max_workers
    # max_workers = MEMORY_LIMIT / (prefetch_factor * batch_mem)
    prefetch_factor = args.prefetch_factor
    
    max_workers_mem = int(MEMORY_LIMIT_BYTES / (prefetch_factor * batch_mem_est))
    # Ensure at least 1 worker if possible, but dont crash
    max_workers_mem = max(0, max_workers_mem)
    
    print(f"Max workers allowed by memory limit ({MEMORY_LIMIT_GB} GB): {max_workers_mem}")
    
    # Select final num_workers
    train_num_workers = min(args.num_workers, max_workers_mem)
    # Drastically reduce val workers since validation is just inference
    val_num_workers = min(2, train_num_workers)
    
    print(f"Using Train Workers: {train_num_workers}")
    print(f"Using Val Workers: {val_num_workers}")

    # Helper to clean args for get_dataloader
    train_dl_kwargs = {
        'num_workers': train_num_workers,
        'pin_memory': args.pin_memory,
        'prefetch_factor': args.prefetch_factor if train_num_workers > 0 else None, # prefetch_factor requires num_workers > 0
        'persistent_workers': args.persistent_workers if train_num_workers > 0 else False
    }
    
    val_dl_kwargs = {
        'num_workers': val_num_workers,
        'pin_memory': args.pin_memory,
        'prefetch_factor': args.prefetch_factor if val_num_workers > 0 else None,
        'persistent_workers': args.persistent_workers if val_num_workers > 0 else False
    }
    
    if train_num_workers == 0:
         print("Warning: num_workers set to 0. Data loading will be on the main process and might be slow.")
    
    
    if args.oversample:
        print("Oversampling enabled. Calculating weights...")
        # Create dataset first to access samples
        train_ds = AVDataset(metadata_file, split='train', data=dataset_data)
        
        # Extract curvatures (index 0 of first future action)
        # s['future_actions'] is list of [curvature, velocity]
        curvatures = []
        if isinstance(train_ds.samples, pd.DataFrame):
            # Optimized pandas access
            # 'future_actions' column contains lists of [curvature, velocity]
            # We can use apply to extract curvatures
            # But checking length first is good.
            # Assuming all samples have valid future_actions or handle empty
            # Pandas vectorization is faster
            def get_curvature(act):
                if len(act) > 0:
                    return act[0][0]
                return 0.0
            curvatures = train_ds.samples['future_actions'].apply(get_curvature).values
        else:
            for s in train_ds.samples:
                # Check shape of future_actions
                # It should be list of lists or numpy array
                acts = s['future_actions']
                if len(acts) > 0:
                    curvatures.append(acts[0][0])
                else:
                    curvatures.append(0.0)
            curvatures = np.array(curvatures)
        
        curvatures = np.array(curvatures)
        
        # Compute histogram to estimate density
        # Use reasonable bins
        bins = np.linspace(-2.0, 2.0, 100)
        hist, bin_edges = np.histogram(curvatures, bins=bins, density=False)
        
        # Avoid division by zero
        hist = hist.astype(float)
        hist[hist == 0] = 1.0 # Should not happen for populated bins, but if sample falls here, we handle below
        
        # Map each sample to a bin index
        bin_indices = np.digitize(curvatures, bin_edges) - 1
        bin_indices = np.clip(bin_indices, 0, len(hist) - 1)
        
        # Compute current probability (density) for each sample
        # p(x) ~ count / total
        current_probs = hist[bin_indices] / len(curvatures)
        
        # Calculate target std from data if not explicitly set?
        # User request: "calculate the stddev based on the training data, rejecting excessive 0 curvature examples"
        # We'll do this calculation and overwrite args.target_curvature_std or use it if not set?
        # Let's perform the calculation.
        epsilon = 0.05
        non_zero_curvatures = curvatures[np.abs(curvatures) > epsilon]
        if len(non_zero_curvatures) > 0:
            calculated_std = np.std(non_zero_curvatures)
            print(f"Calculated standard deviation of non-zero (> {epsilon}) curvatures: {calculated_std}")
            
            args.target_curvature_std = float(calculated_std)
        else:
            print("Warning: No non-zero curvatures found. Using default target std.")

        # Compute target probability
        # q(x) ~ Normal(0, target_std)
        target_std = args.target_curvature_std
        target_probs = np.exp(-0.5 * (curvatures / target_std) ** 2) / (target_std * np.sqrt(2 * np.pi))
        
        # Weight w = q(x) / p(x)
        # Normalize weights so they sum to N? Not strictly necessary for sampler, but good for debugging.
        sample_weights = target_probs / current_probs
        sample_weights = torch.DoubleTensor(sample_weights)
        
        # Create Sampler
        # num_samples = len(dataset) to keep epoch size same
        train_sampler = torch.utils.data.WeightedRandomSampler(sample_weights, num_samples=len(train_ds), replacement=True)
        
        # Create DataLoader with sampler
        # shuffle must be False
        train_loader = torch.utils.data.DataLoader(train_ds, batch_size=args.batch_size, shuffle=False, 
                                                 sampler=train_sampler, **train_dl_kwargs)
        
        print(f"Oversampling initialized. Target Std: {target_std}")
    else:
        train_loader = get_dataloader(metadata_file, split='train', batch_size=args.batch_size, shuffle=True, **train_dl_kwargs, dataset_data=dataset_data)
        
    val_loader = get_dataloader(metadata_file, split='val', batch_size=args.batch_size, shuffle=False, **val_dl_kwargs, dataset_data=dataset_data)
    
    # Inspect one sample to determine input/output shapes (we already have one from temp_loader)
    # Re-use sample_batch but we need to check if dimensions match what model expects (B, T, C, H, W)
    # temp_loader had batch_size=1, so we should be careful.
    # The shapes in sample_batch are (1, ...).
    # Model expects (B, ...).
    
    input_shape = list(sample_batch['image'].shape)
    input_shape[0] = args.batch_size # Pretend it is the full batch size for logging
    input_shape = torch.Size(input_shape)
    
    print(f"Input Shape (simulated): {input_shape}")
    action_shape = sample_batch['action'].shape 
    print(f"Action Shape (from sample): {action_shape}")
    
    if len(input_shape) == 5:
        B, T, C, H, W = input_shape
    else:
        # Fallback for legacy or unexpected shapes
        # Assuming (B, C, H, W) where C includes time
        raise ValueError(f"Expected 5D input (B, T, C, H, W), got {input_shape}")
    
    output_steps = action_shape[1]
    input_channels = C
    
    print(f"Num Frames (T): {T}")
    print(f"Channels per Frame (C): {C}")
    print(f"Output Steps: {output_steps}")
    
    # Initialize Model
    if args.model == 'cnn':
        # CNN flattens T into channels
        model = CNN(T * C, output_steps, dropout=args.dropout)
    elif args.model == 'mlp':
        # MLP flattens T into channels/input dim
        model = MLP(T * C, output_steps, dropout=args.dropout)
    elif args.model == 'transformer':
        # Transformer handles T internally, takes channels per frame
        model = Transformer(C, output_steps, dropout=args.dropout)
    else:
        raise ValueError(f"Unknown model type: {args.model}")
        
    model = model.to(device)
    
    # Compute Statistics
    mean_stats, std_stats, act_mean_stats, act_std_stats = compute_dataset_stats(train_loader)
    
    # Create tensors for normalization
    # Shape for broadcasting: (1, 1, C, 1, 1)
    mean_tensor = torch.tensor(mean_stats, device=device).view(1, 1, -1, 1, 1)
    std_tensor = torch.tensor(std_stats, device=device).view(1, 1, -1, 1, 1)
    
    # Shape for action normalization: (1, 1, 2)
    act_mean_tensor = torch.tensor(act_mean_stats, device=device).view(1, 1, 2)
    act_std_tensor = torch.tensor(act_std_stats, device=device).view(1, 1, 2)
    
    print(f"Normalization enabled. Image Mean: {mean_stats}, Std: {std_stats}")
    print(f"Action Normalization enabled. Mean: {act_mean_stats}, Std: {act_std_stats}")

    # Optimizer and Loss
    optimizer = optim.Adam(model.parameters(), lr=args.lr)

    criterion = nn.MSELoss()
    
    # Tensorboard
    writer = SummaryWriter(log_dir=log_dir)
    
    # Training Loop
    best_val_loss = float('inf')
    patience_counter = 0
    
    for epoch in range(args.epochs):
        model.train()
        train_loss = 0.0
        
        pbar = tqdm(train_loader, desc=f'Epoch {epoch+1}/{args.epochs}')
        for i, batch in enumerate(pbar):
            images = batch['image'].to(device)
            actions = batch['action'].to(device)
            
            # Normalize Inputs
            images = (images - mean_tensor) / std_tensor
            
            # Normalize Targets
            actions = (actions - act_mean_tensor) / act_std_tensor
            
            optimizer.zero_grad()
            outputs = model(images)
            
            if args.weighted_loss:
                # Weighted MSE Loss
                # Weights based on curvature magnitude (index 0)
                # Ensure actions are normalized before this? Yes they are.
                # Use raw magnitude or normalized magnitude? Normalized is fine, high absolute value = extreme.
                # However, normalized values can be negative. We use abs().
                
                # Curvature is index 0.
                curvature = actions[:, :, 0]
                # Weight = 1 + alpha * |curvature|
                weights = 1.0 + args.weighted_loss_alpha * torch.abs(curvature)
                
                # Calculate raw MSE per element
                loss_raw = F.mse_loss(outputs, actions, reduction='none') # (B, T, 2)
                
                # Apply weights
                # Weights (B, T) -> (B, T, 1) to broadcast to velocity as well
                loss = (loss_raw * weights.unsqueeze(-1)).mean()
            else:
                loss = criterion(outputs, actions)
            
            loss.backward()
            optimizer.step()
            
            train_loss += loss.item()
            
            # Update progress bar with current loss
            pbar.set_postfix({'loss': f'{loss.item():.4f}'})
            
        avg_train_loss = train_loss / len(train_loader)
        
        # Validation
        model.eval()
        val_loss = 0.0
        with torch.no_grad():
            for batch in val_loader:
                images = batch['image'].to(device)
                actions = batch['action'].to(device)
                
                # Normalize Inputs
                images = (images - mean_tensor) / std_tensor
                
                # Normalize Targets
                actions = (actions - act_mean_tensor) / act_std_tensor
                
                outputs = model(images)
                loss = criterion(outputs, actions)
                val_loss += loss.item()
                
        if len(val_loader) > 0:
            avg_val_loss = val_loss / len(val_loader)
        else:
            avg_val_loss = 0.0
        
        print(f"Epoch [{epoch+1}/{args.epochs}] Train Loss: {avg_train_loss:.4f} Val Loss: {avg_val_loss:.4f}")
        
        # Logging
        writer.add_scalar('Loss/train', avg_train_loss, epoch)
        writer.add_scalar('Loss/val', avg_val_loss, epoch)

        # Ray Tune Reporting
        if ray.is_initialized():
             # We report loss to Ray Tune. 
             # Note: logic should handle if not in a session, but ray.train.report checks internally or raises error.
             try:
                 tune.report({"loss": avg_val_loss})
             except RuntimeError:
                 pass # Not inside a tune session
        
        
        # Early Stopping and Checkpointing
        if avg_val_loss < best_val_loss:
            best_val_loss = avg_val_loss
            patience_counter = 0
            # Save best model
            model_path = os.path.join(models_dir, "best_model.pth")
            torch.save(model.state_dict(), model_path)
            print(f"Saved best model to {model_path}")
            
            # Save training metadata
            # Save training metadata
            
            # Load dataset metadata to get color space and rates
            with open(metadata_file, 'r') as f:
                ds_meta = json.load(f)
            ds_params = ds_meta.get('parameters', {})
            color_space = ds_params.get('channels', 'rgb')
            history_rate = ds_params.get('history_rate')
            future_rate = ds_params.get('future_rate')
            
            training_meta = {
                'model_type': args.model,
                'dataset': args.dataset,
                'input_channels': input_channels, # This is usually per-frame * n_frames or just total channels
                'mean': mean_stats,
                'std': std_stats,
                'action_mean': act_mean_stats,
                'action_std': act_std_stats,
                'input_height': input_shape[3], # (B, T, C, H, W) -> H is index 3
                'input_width': input_shape[4],  # (B, T, C, H, W) -> W is index 4
                'history_frames': T,
                'future_frames': output_steps,
                'color_space': color_space,
                'history_rate': history_rate,
                'future_rate': future_rate,
                'dropout': args.dropout,
                'dropout': args.dropout,
                'weighted_loss': args.weighted_loss,
                'weighted_loss_alpha': args.weighted_loss_alpha,
                'oversample': args.oversample,
                'target_curvature_std': args.target_curvature_std,
                'best_val_loss': best_val_loss,
                'epoch': epoch + 1,
            }
            meta_path = os.path.join(models_dir, "training_metadata.json")
            with open(meta_path, 'w') as f:
                json.dump(training_meta, f, indent=2)
            print(f"Saved training metadata to {meta_path}")
        else:
            patience_counter += 1
            
        if patience_counter >= args.early_stopping_epochs:
            print(f"Early stopping triggered after {epoch+1} epochs.")
            break
            
    writer.close()
    print("Training finished.")

def train_wrapper(config, base_args, dataset_data=None, cpu_resources=None):
    """
    Wrapper to convert Ray Tune config to args and call train.
    """
    import copy
    # Deep copy base args to avoid modifying it for other trials (though processes are usually separate)
    args = copy.deepcopy(base_args)
    
    # Inject dataset_data if provided via with_parameters
    if dataset_data is not None:
        args.dataset_data = dataset_data
    
    # Update args with config values
    for k, v in config.items():
        # Handle hyphen vs underscore
        k_normalized = k.replace('-', '_')
        if hasattr(args, k_normalized):
             setattr(args, k_normalized, v)
        else:
             print(f"Warning: Config key {k} ({k_normalized}) is not a valid argument.")

    # Dynamic Worker Scaling
    # Use explicitly passed cpu_resources if available
    if cpu_resources is not None:
        cpu_alloc = cpu_resources
        # We need one CPU for the training loop itself (main thread), 
        # remaining can be workers.
        adjusted_workers = int(max(0, cpu_alloc - 1))
        
        if args.num_workers > adjusted_workers:
            print(f"Ray Tune: Adjusting num_workers from {args.num_workers} to {adjusted_workers} to match allocated CPU resources ({cpu_alloc}).")
            args.num_workers = adjusted_workers
    
    # Run training
    train(args)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Train Car Action Prediction Model')
    
    parser.add_argument('--model', type=str, required=True, choices=['cnn', 'mlp', 'transformer'], help='Model architecture to use')
    parser.add_argument('--dataset', type=str, required=True, help='Dataset name (without .json extension)')
    parser.add_argument('--batch-size', type=int, default=32, help='Batch size')
    parser.add_argument('--lr', type=float, default=1e-4, help='Learning rate')
    parser.add_argument('--dropout', type=float, default=0.1, help='Dropout rate')
    parser.add_argument('--epochs', type=int, default=100, help='Maximum number of epochs')
    parser.add_argument('--early-stopping-epochs', type=int, default=10, help='Number of epochs to wait for improvement before early stopping')
    
    # Data Loading Optimizations
    parser.add_argument('--num-workers', type=int, default=min(os.cpu_count() or 4, 16), help='Number of data loading workers')
    parser.add_argument('--pin-memory', action='store_true', help='Pin memory for faster host-to-device transfer')
    parser.add_argument('--no-pin-memory', dest='pin_memory', action='store_false')
    parser.set_defaults(pin_memory=True)
    parser.add_argument('--prefetch-factor', type=int, default=2, help='Number of batches loaded in advance by each worker')
    parser.add_argument('--persistent-workers', action='store_true', help='Keep workers alive between epochs')
    parser.add_argument('--no-persistent-workers', dest='persistent_workers', action='store_false')
    parser.set_defaults(persistent_workers=True)
    
    parser.set_defaults(persistent_workers=True)
    
    parser.add_argument('--weighted-loss', action='store_true', help='Apply curvature-based weighting to loss')
    parser.add_argument('--weighted-loss-alpha', type=float, default=2.0, help='Alpha parameter for weighted loss (weight = 1 + alpha * |curvature|)')

    parser.add_argument('--oversample', action='store_true', help='Oversample high-curvature data')
    parser.add_argument('--target-curvature-std', type=float, default=1.0, help='Target standard deviation for curvature distribution when oversampling')
    
    parser.add_argument('--ray-grid-search', type=str, help='JSON string specifying hyperparameter grid search (e.g., \'{"batch-size": [16, 32], "lr": [1e-3, 1e-4], "dropout": [0.1, 0.2], "weighted-loss-alpha": [1.0, 2.0]}\')')

    parser.add_argument('--ray-resources', type=str, default='{"cpu": 2, "gpu": 1.0}', help='JSON string specifying resources per trial (e.g., \'{"cpu": 2, "gpu": 1.0}\')')

    args = parser.parse_args()

    if args.ray_grid_search:
        # Ray Grid Search Mode
        print(f"Starting Ray Grid Search with params: {args.ray_grid_search}")
        
        try:
            search_params = json.loads(args.ray_grid_search)
        except json.JSONDecodeError as e:
            print(f"Error parsing --ray-grid-search JSON: {e}")
            exit(1)

        try:
            resources_per_trial = json.loads(args.ray_resources)
        except json.JSONDecodeError as e:
            print(f"Error parsing --ray-resources JSON: {e}")
            exit(1)

        print(f"Resources per trial: {resources_per_trial}")

        # Initialize Ray
        ray.init(ignore_reinit_error=True)
        
        # Load Dataset Once for Shared Memory
        dataset_dir = os.path.expanduser('/var/local/sw8565/data/datasets')
        metadata_file = os.path.join(dataset_dir, f"{args.dataset}.json")
        
        if not os.path.exists(metadata_file):
            print(f"Error: Dataset metadata file not found during pre-load: {metadata_file}")
            # We don't exit here, we let train() fail or we can exit.
            # But let's load it.
            exit(1)
            
        print(f"Pre-loading dataset metadata from {metadata_file} to Ray Object Store...")
        with open(metadata_file, 'r') as f:
            dataset_dict = json.load(f)
            
        # Optimize: Convert samples to Pandas DataFrame
        print("Converting samples to Pandas DataFrame for memory efficiency...")
        dataset_dict['samples'] = pd.DataFrame(dataset_dict['samples'])
        
        # Put in Object Store
        # Ray automatically handles large objects when passed to remote functions/actors, 
        # but with_parameters ensures it's put in the store and passed as object ref.
        # Actually verify: tune.with_parameters replaces the value with the object ref in the config dict.
        # So 'dataset_data' key in config will have the actual data (deserialized) or the ref?
        # "The parameters are automatically put into the Ray object store. The function will receive the dereferenced objects."
        # Perfect.
        
        # Build Grid Search Config
        param_space = {}
        for key, values in search_params.items():
            if not isinstance(values, list):
                print(f"Error: Value for {key} must be a list of values to search over.")
                exit(1)
            param_space[key] = tune.grid_search(values)
            
        print(f"Parameter Space: {param_space}")
        
        # Create Tuner
        # We need to pass 'args' to the wrapper. We can use tune.with_parameters.
        # But wait, 'param_space' defines the variable parts. 'args' provides the fixed parts.
        
        # Determine CPU resources per trial to pass to wrapper
        cpu_per_trial = resources_per_trial.get("cpu", 0) + resources_per_trial.get("CPU", 0)
        
        # Apply resources
        trainable = tune.with_resources(
            tune.with_parameters(train_wrapper, base_args=args, dataset_data=dataset_dict, cpu_resources=cpu_per_trial),
            resources=resources_per_trial
        )
        
        tuner = tune.Tuner(
            trainable,
            param_space=param_space,
            run_config=tune.RunConfig(
                name=f"tune_{args.dataset}_{int(time.time())}",
                storage_path=os.path.expanduser("~/roboracer_ws/data/ray_results")
            ),
             tune_config=tune.TuneConfig(
                metric="loss",
                mode="min",
            )
        )
        
        results = tuner.fit()
        
        print("Grid Search Completed.")
        best_result = results.get_best_result(metric="loss", mode="min")
        print(f"Best Result: {best_result.config}")
        print(f"Best Loss: {best_result.metrics.get('loss')}")
        
    else:
        # Normal Training Mode
        train(args)
