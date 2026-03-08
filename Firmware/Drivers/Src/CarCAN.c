#include "CarCAN.h"

FDCAN_HandleTypeDef CarCAN;

void CarCAN_Init(void)
{

    CarCAN.Instance = FDCAN1;
    CarCAN.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    CarCAN.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    CarCAN.Init.Mode = FDCAN_MODE_NORMAL;
    CarCAN.Init.AutoRetransmission = DISABLE;
    CarCAN.Init.TransmitPause = DISABLE;
    CarCAN.Init.ProtocolException = DISABLE;
    CarCAN.Init.NominalPrescaler = 16;
    CarCAN.Init.NominalSyncJumpWidth = 1;
    CarCAN.Init.NominalTimeSeg1 = 1;
    CarCAN.Init.NominalTimeSeg2 = 1;
    CarCAN.Init.DataPrescaler = 1;
    CarCAN.Init.DataSyncJumpWidth = 1;
    CarCAN.Init.DataTimeSeg1 = 1;
    CarCAN.Init.DataTimeSeg2 = 1;
    CarCAN.Init.StdFiltersNbr = 0;
    CarCAN.Init.ExtFiltersNbr = 0;
    CarCAN.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
    if (HAL_FDCAN_Init(&CarCAN) != HAL_OK)
    {
        Error_Handler();
    }
}

void CarCAN_Send(void);

void CarCAN_Recieve(void);

// HAL_FDCAN_MspInit and MspDeInit are defined in ElconCAN.c (handles both FDCAN1 and FDCAN3)