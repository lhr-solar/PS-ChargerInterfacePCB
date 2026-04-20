#include "CarCAN.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "CAN_FD.h"

static FDCAN_HandleTypeDef *CarCAN = NULL;

static FDCAN_TxHeaderTypeDef carCAN_tx_header = {
    .Identifier = CAN_ID_BPS_STATUS,
    .IdType = FDCAN_STANDARD_ID,
    .TxFrameType = FDCAN_DATA_FRAME,
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

can_status_t CarCAN_Send(uint32_t id, uint8_t data[8], uint32_t dlc, TickType_t delay_ticks)
{
    carCAN_tx_header.Identifier = id;
    carCAN_tx_header.DataLength = dlc;

    if (can_fd_send(CarCAN, &carCAN_tx_header, data, delay_ticks) == CAN_ERR)
    {
        return CAN_ERR;
    }

    return CAN_OK;
}

can_status_t CarCAN_Receive_BPS_Status(uint8_t data[8], TickType_t delay_ticks)
{
    return can_fd_recv(CarCAN, CAN_ID_BPS_STATUS, &carCAN_rx_header, data, delay_ticks);
}

can_status_t CarCAN_Receive_BPS_Voltage(uint8_t data[8], TickType_t delay_ticks)
{
    return can_fd_recv(CarCAN, CAN_ID_BPS_VOLTAGE_AGGREGATE_ARR, &carCAN_rx_header, data, delay_ticks);
}

can_status_t CarCAN_Send_ChargerInterface_Status(uint16_t output_voltage_dv, uint16_t output_current_da, uint8_t elcon_comm_ok, uint8_t elcon_fault, TickType_t delay_ticks)
{
    uint8_t data[8] = {0};

    // bytes 0-1: output voltage, uint16 little-endian, scale 0.1V
    data[0] = (uint8_t)(output_voltage_dv & 0xFF);
    data[1] = (uint8_t)(output_voltage_dv >> 8);

    // bytes 2-3: output current, uint16 little-endian, scale 0.1A
    data[2] = (uint8_t)(output_current_da & 0xFF);
    data[3] = (uint8_t)(output_current_da >> 8);

    // byte 4: bit 0 = elcon_comm_ok, bit 1 = elcon_fault
    data[4] = (elcon_comm_ok & 0x01) | ((elcon_fault & 0x01) << 1);

    // bytes 5-7: reserved, already zeroed

    //TODO: update with generated CAN ID once done with Interface DBC

    return CarCAN_Send(CAN_ID_CHARGERINTERFACE_STATUS, data, FDCAN_DLC_BYTES_5, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
}

// TODO: test this with real BPS data

void CarCAN_Unpack_BPS_Aggregate(const uint8_t data[8], CarCAN_BPS_Aggregate_t *agg)
{
    uint8_t idx = data[0] & 0x1F;
    if (idx >= BPS_TAP_COUNT)
    {
        return;
    }

    // BPS_Voltage_Tap_Data: start bit 8, length 16, scale 0.001V
    uint16_t raw_v = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
    agg->taps[idx].voltage_mv = raw_v;

    // BPS_Temperature_Tap_Data: start bit 24, length 32, scale 0.001°C
    int32_t raw_t = (int32_t)data[3] | ((int32_t)data[4] << 8) | ((int32_t)data[5] << 16) | ((int32_t)data[6] << 24);
    agg->taps[idx].temperature_mdc = raw_t;

    agg->last_updated_idx = idx;
}
