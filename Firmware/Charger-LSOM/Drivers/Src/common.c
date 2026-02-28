#include "common.h"
#include "StatusLED.h"

EventGroupHandle_t faultStateBits;
StaticEventGroup_t faultStateBitsBuffer;

uint8_t faultBits_init(void)
{
    faultStateBits = xEventGroupCreateStatic(&faultStateBitsBuffer);
    if (faultStateBits == NULL)
    {
        return 0;
    }
    return 1;
}

void faultBits_set(fault_state_t bit)
{
    if (bit >= NUM_FAULTS)
    {
        return;
    }

    xEventGroupSetBits(faultStateBits, FAULT_BIT(bit));

    return;
}

bool faultBit_wait(fault_state_t fault, TickType_t xTicksToWait)
{
    if (fault >= NUM_FAULTS)
    {
        return false;
    }

    EventBits_t bits = xEventGroupWaitBits(faultStateBits, FAULT_BIT(fault), pdFALSE, pdFALSE, xTicksToWait);
    return (bits & FAULT_BIT(fault)) != 0;
}

bool faultBits_isSet(fault_state_t fault)
{
    if (fault >= NUM_FAULTS)
    {
        return false;
    }

    return (xEventGroupGetBits(faultStateBits) & FAULT_BIT(fault)) != 0;
}

void faultBits_clear(fault_state_t fault)
{
    if (fault >= NUM_FAULTS)
    {
        return;
    }

    xEventGroupClearBits(faultStateBits, FAULT_BIT(fault));
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 10;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

void Error_Handler(void)
{

    Fault_Indicator(true);
    HAL_TIM_PWM_MspDeInit(&htim5); // stop buzzer PWM

    // TODO: send CAN messages about fault for telemetry, later

    while (1)
    {
    }
}
