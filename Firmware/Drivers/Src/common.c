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


void Error_Handler(void)
{

    LED_State_t leds = {
        .evse_present = false,
        .charging = false,
        .fault = true,
        .hv_active = false,
    };
    LEDSet(&leds);
    HAL_TIM_PWM_MspDeInit(&htim5); // stop buzzer PWM

    // TODO: send CAN messages about fault for telemetry, later

    while (1)
    {
    }
}

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED = 0;

// Single MspInit handles both FDCAN instances — HAL only allows one definition project-wide
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *fdcanHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    // Shared clock source config (only needs to run once, same source for all FDCAN)
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_RCC_FDCAN_CLK_ENABLED++;
    if (HAL_RCC_FDCAN_CLK_ENABLED == 1)
    {
        __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();

    if (fdcanHandle->Instance == FDCAN3)
    {
        HAL_NVIC_SetPriority(FDCAN3_IT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(FDCAN3_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN3_IT1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(FDCAN3_IT1_IRQn);
        // ElconCAN: PA8 (RX), PA15 (TX), AF11
        GPIO_InitStruct.Pin = ELCONCAN_TX_PIN | ELCONCAN_RX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF11_FDCAN3;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    else if (fdcanHandle->Instance == FDCAN1)
    {
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
        HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
        HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
        // CarCAN: PA11 (RX), PA12 (TX), AF9
        GPIO_InitStruct.Pin = CARCAN_RX_PIN | CARCAN_TX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *fdcanHandle)
{
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if (HAL_RCC_FDCAN_CLK_ENABLED == 0)
    {
        __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    if (fdcanHandle->Instance == FDCAN3)
    {
        HAL_GPIO_DeInit(GPIOA, ELCONCAN_TX_PIN | ELCONCAN_RX_PIN);
    }
    else if (fdcanHandle->Instance == FDCAN1)
    {
        HAL_GPIO_DeInit(GPIOA, CARCAN_RX_PIN | CARCAN_TX_PIN);
    }
}
