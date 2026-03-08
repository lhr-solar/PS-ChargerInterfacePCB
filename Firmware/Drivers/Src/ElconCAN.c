#include <stdint.h>
#include "stm32xx_hal.h"
#include "ElconCAN.h"
#include "CAN_FD.h"

FDCAN_HandleTypeDef *ElconCAN = NULL;

static FDCAN_TxHeaderTypeDef elcon_tx_header = {
    .Identifier = ELCONCAN_TX_ID,
    .IdType = FDCAN_EXTENDED_ID,
    .TxFrameType = FDCAN_DATA_FRAME,
    .DataLength = FDCAN_DLC_BYTES_8,
    .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
    .BitRateSwitch = FDCAN_BRS_OFF,
    .FDFormat = FDCAN_CLASSIC_CAN,
    .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
    .MessageMarker = 0,
};

static FDCAN_RxHeaderTypeDef elcon_rx_header;

can_status_t ElconCAN_Init(void)
{

    ElconCAN = hfdcan3;

    ElconCAN->Instance = FDCAN3;
    ElconCAN->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    ElconCAN->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    ElconCAN->Init.Mode = FDCAN_MODE_NORMAL;
    ElconCAN->Init.AutoRetransmission = DISABLE;
    ElconCAN->Init.TransmitPause = DISABLE;
    ElconCAN->Init.ProtocolException = DISABLE;
    // 80 MHz PCLK1, 250 kbps: (1 + 14 + 5) * 16 / 80MHz = 4us, 75% sample point
    ElconCAN->Init.NominalPrescaler = 20;
    ElconCAN->Init.NominalSyncJumpWidth = 1;
    ElconCAN->Init.NominalTimeSeg1 = 13;
    ElconCAN->Init.NominalTimeSeg2 = 2;
    ElconCAN->Init.DataPrescaler = 1;
    ElconCAN->Init.DataSyncJumpWidth = 1;
    ElconCAN->Init.DataTimeSeg1 = 1;
    ElconCAN->Init.DataTimeSeg2 = 1;
    ElconCAN->Init.StdFiltersNbr = 0;
    ElconCAN->Init.ExtFiltersNbr = 1;
    ElconCAN->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    // only accept CAN ID: 0x1806E5F4
    FDCAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

    // Incoming Elcon to board ID: 0x18FF50E5
    sFilterConfig.FilterID1 = ELCONCAN_RX_ID;
    sFilterConfig.FilterID2 = 0x1FFFFFFF;

    if (can_fd_init(ElconCAN, &sFilterConfig) != CAN_OK)
    {
        return CAN_ERR;
    }

    if (can_fd_start(ElconCAN) != CAN_OK)
    {
        return CAN_ERR;
    }

    return CAN_OK;
}

/*
 * Packs the 8-byte payload for CAN ID 0x1806E5F4
 * target_voltage_v: Desired voltage in Volts (e.g., 320.1)
 * target_current_a: Desired current in Amps (e.g., 58.2)
 * stop_charging: 0 = Open/Charging, 1 = BPS/stop
 * payload: Pointer to an 8-byte array to store the result
 */

can_status_t ElconCAN_Send(float target_voltage_v, float target_current_a, uint8_t stop_charging, TickType_t delay_ticks)
{

    uint8_t payload[8] = {0};
    uint16_t v_scaled = (uint16_t)(target_voltage_v * 10.0f);
    uint16_t c_scaled = (uint16_t)(target_current_a * 10.0f);

    payload[0] = (uint8_t)(v_scaled >> 8);
    payload[1] = (uint8_t)(v_scaled & 0xFF);
    payload[2] = (uint8_t)(c_scaled >> 8);
    payload[3] = (uint8_t)(c_scaled & 0xFF);
    payload[4] = stop_charging ? 1 : 0;
    payload[5] = 0;
    payload[6] = 0;
    payload[7] = 0;

    if (can_fd_send(ElconCAN, &elcon_tx_header, payload, delay_ticks) == CAN_ERR)
    {
        return CAN_ERR;
    }

    HAL_GPIO_TogglePin(LED_EVSE_PORT, LED_EVSE_PIN);

    return CAN_OK;
}

can_status_t ElconCAN_Recieve(ElconStatus_t *status, uint32_t id, uint8_t data[], TickType_t delay_ticks)
{

    if (can_fd_recv(ElconCAN, id, &elcon_rx_header, data, delay_ticks) != CAN_OK)
    {
        return CAN_ERR;
    }

    uint16_t v_raw = ((uint16_t)data[0] << 8) | data[1];
    uint16_t c_raw = ((uint16_t)data[2] << 8) | data[3];

    // Reverse the scaling to get real Volts and Amps
    status->output_voltage = (float)v_raw / 10.0f;
    status->output_current = (float)c_raw / 10.0f;

    // Bit 0: 1 = Hardware Failure
    status->flag_hw_failure = (data[4] & 0x01) ? 1 : 0;

    // Bit 1: 1 = Over temperature protection
    status->flag_over_temp = (data[4] & 0x02) ? 1 : 0;

    // Bit 2: 1 = Input voltage is wrong
    status->flag_input_voltage_wrong = (data[4] & 0x04) ? 1 : 0;

    // Bit 3: 1 = Charger stays closed to prevent reverse polarity
    // 0 = charger open
    status->flag_starting_state = (data[4] & 0x08) ? 1 : 0;

    // Bit 4: 1 = Communication receive time-out
    status->flag_comm_timeout = (data[4] & 0x10) ? 1 : 0;

    HAL_GPIO_TogglePin(LED_CHARGE_PORT, LED_CHARGE_PIN);

    return CAN_OK;
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