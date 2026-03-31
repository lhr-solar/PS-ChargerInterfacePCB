#include "CarCAN.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "CAN_FD.h"

static FDCAN_HandleTypeDef *CarCAN = NULL;

static FDCAN_TxHeaderTypeDef carCAN_tx_header = {
    .Identifier = BPS_Status_ID,
    .IdType = FDCAN_STANDARD_ID,
    .TxFrameType = FDCAN_DATA_FRAME,
    .DataLength = FDCAN_DLC_BYTES_8,
    .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
    .BitRateSwitch = FDCAN_BRS_OFF,
    .FDFormat = FDCAN_CLASSIC_CAN,
    .TxEventFifoControl = FDCAN_NO_TX_EVENTS,
    .MessageMarker = 0,
};

static FDCAN_RxHeaderTypeDef carCAN_rx_header;

can_status_t CarCAN_Init(void)
{
    CarCAN = hfdcan3;

    CarCAN->Instance = FDCAN3;
    CarCAN->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    CarCAN->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    CarCAN->Init.Mode = FDCAN_MODE_NORMAL;
    CarCAN->Init.AutoRetransmission = DISABLE;
    CarCAN->Init.TransmitPause = DISABLE;
    CarCAN->Init.ProtocolException = DISABLE;
    CarCAN->Init.NominalPrescaler = 20;
    CarCAN->Init.NominalSyncJumpWidth = 1;
    CarCAN->Init.NominalTimeSeg1 = 13;
    CarCAN->Init.NominalTimeSeg2 = 2;
    CarCAN->Init.DataPrescaler = 1;
    CarCAN->Init.DataSyncJumpWidth = 1;
    CarCAN->Init.DataTimeSeg1 = 1;
    CarCAN->Init.DataTimeSeg2 = 1;
    CarCAN->Init.StdFiltersNbr = 0;
    CarCAN->Init.ExtFiltersNbr = 0;
    CarCAN->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    FDCAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

    // no filter yet
    sFilterConfig.FilterID1 = 0x00000000;
    sFilterConfig.FilterID2 = 0x00000000;

    if (can_fd_init(CarCAN, &sFilterConfig) != CAN_OK)
    {
        return CAN_ERR;
    }

    if (can_fd_start(CarCAN) != CAN_OK)
    {
        return CAN_ERR;
    }

    return CAN_OK;
}

can_status_t CarCAN_Send(uint32_t id, uint8_t data[8], TickType_t delay_ticks)
{
    carCAN_tx_header.Identifier = id;

    if (can_fd_send(CarCAN, &carCAN_tx_header, data, delay_ticks) == CAN_ERR)
    {
        HAL_GPIO_WritePin(LED_HV_PORT, LED_HV_PIN, GPIO_PIN_RESET);
        return CAN_ERR;
    }

    HAL_GPIO_WritePin(LED_HV_PORT, LED_HV_PIN, GPIO_PIN_SET);
    return CAN_OK;
}

can_status_t CarCAN_Receive(uint32_t *id_out, uint8_t data[8], TickType_t delay_ticks)
{
    // reads through both BPS Status ID and Aggregated Arr ID
    static const uint32_t bps_ids[] = {BPS_Status_ID, BPS_Aggregate_Arr_ID};
    for (int i = 0; i < (int)(sizeof(bps_ids) / sizeof(bps_ids[0])); i++)
    {
        TickType_t ticks = (i == (int)(sizeof(bps_ids) / sizeof(bps_ids[0])) - 1) ? delay_ticks : 0;
        can_status_t result = can_fd_recv(CarCAN, bps_ids[i], &carCAN_rx_header, data, ticks);
        if (result == CAN_OK)
        {
            *id_out = bps_ids[i];
            return CAN_OK;
        }
    }
    return CAN_EMPTY;
}

// TODO: test this with real BPS data

void CarCAN_Unpack_BPS_Aggregate(const uint8_t data[8], CarCAN_BPS_Aggregate_t *agg)
{
    uint8_t idx = data[0] & 0x1F;
    if (idx >= BPS_TAP_COUNT)
    {
        return;
    }

    // BPS_Voltage_Tap_Data: start bit 8, length 16, scale 0.001
    uint16_t raw_v = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
    agg->taps[idx].voltage = raw_v * 0.001f;

    // BPS_Temperature_Tap_Data: start bit 24, length 32, scale 0.001
    int32_t raw_t = (int32_t)data[3] | ((int32_t)data[4] << 8) | ((int32_t)data[5] << 16) | ((int32_t)data[6] << 24);
    agg->taps[idx].temperature = raw_t * 0.001f;

    agg->last_updated_idx = idx;
}
