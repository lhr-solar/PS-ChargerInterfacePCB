#include <stdint.h>
#include "stm32xx_hal.h"
#include "ElconCAN.h"
#include "CAN_FD.h"
#include "common.h"

static FDCAN_HandleTypeDef *ElconCAN = NULL;

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

    ElconCAN = hfdcan1;

    ElconCAN->Instance = FDCAN1;
    ElconCAN->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    ElconCAN->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    ElconCAN->Init.Mode = FDCAN_MODE_NORMAL;
    ElconCAN->Init.AutoRetransmission = DISABLE;
    ElconCAN->Init.TransmitPause = DISABLE;
    ElconCAN->Init.ProtocolException = DISABLE;
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

    // TODO: accept only 1 CAN ID and test that with HW later

    // only accept CAN ID: 0x1806E5F4 (add back later)
    FDCAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;

    // accept only 0x18FF50E5 — all 29 extended ID bits must match
    sFilterConfig.FilterID1 = 0x18FF50E5;  // ID to match
    sFilterConfig.FilterID2 = 0x1FFFFFFF;  // mask: all 29 bits compared

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

can_status_t ElconCAN_Send(uint32_t id, uint8_t data[8], TickType_t delay_ticks)
{
    elcon_tx_header.Identifier = id;

    if (can_fd_send(ElconCAN, &elcon_tx_header, data, delay_ticks) == CAN_ERR)
    {
        HAL_GPIO_WritePin(LED_HV_PORT, LED_HV_PIN, GPIO_PIN_RESET);
        return CAN_ERR;
    }

    HAL_GPIO_WritePin(LED_HV_PORT, LED_HV_PIN, GPIO_PIN_SET);
    return CAN_OK;
}

can_status_t ElconCAN_Receive(ElconStatus_t *status, uint32_t id, uint8_t data[], TickType_t delay_ticks)
{

    can_status_t result = can_fd_recv(ElconCAN, id, &elcon_rx_header, data, delay_ticks);
    if (result == CAN_EMPTY)
    {
        return CAN_EMPTY;
    }
    if (result != CAN_OK)
    {
        return CAN_ERR;
    }

    // checks for at least 5 bytes recieved
    if (elcon_rx_header.DataLength < FDCAN_DLC_BYTES_5)
    {
        return CAN_ERR;
    }

    // byte 1/2 = actual voltage output
    uint16_t v_raw = ((uint16_t)data[0] << 8) | data[1];

    // byte 3/4 = actual current output
    uint16_t c_raw = ((uint16_t)data[2] << 8) | data[3];

    // decode to get real Volts and Amps
    status->output_voltage = (float)v_raw / 10.0f;
    status->output_current = (float)c_raw / 10.0f;

    // byte 5, all status flags
    //  Bit 0: 1 = Hardware Failure
    status->flag_hw_failure = (data[4] & 0x01) != 0;

    // Bit 1: 1 = Over temperature protection
    status->flag_over_temp = (data[4] & 0x02) != 0;

    // Bit 2: 1 = Input voltage is wrong (charger stops)
    status->flag_input_voltage_wrong = (data[4] & 0x04) != 0;

    // Bit 3: 1 = Charger stays closed to prevent reverse polarity
    status->flag_starting_state = (data[4] & 0x08) != 0;

    // Bit 4: 1 = Communication receive time-out (bad bad stuff)
    status->flag_comm_timeout = (data[4] & 0x10) != 0;

    return CAN_OK;
}
