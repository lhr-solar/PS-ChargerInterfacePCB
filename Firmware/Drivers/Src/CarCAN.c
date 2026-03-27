#include "CarCAN.h"
#include "stm32xx_hal.h"
#include "ElconCAN.h"
#include "common.h"
#include "CAN_FD.h"

static FDCAN_HandleTypeDef *CarCAN = NULL;

static FDCAN_TxHeaderTypeDef carCAN_tx_header = {
    .Identifier = ELCONCAN_TX_ID,
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

void CarCAN_Init(void)
{
    CarCAN = hfdcan3;

    CarCAN->Instance = FDCAN3;
    CarCAN->Init.ClockDivider = FDCAN_CLOCK_DIV1;
    CarCAN->Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    CarCAN->Init.Mode = FDCAN_MODE_NORMAL;
    CarCAN->Init.AutoRetransmission = DISABLE;
    CarCAN->Init.TransmitPause = DISABLE;
    CarCAN->Init.ProtocolException = DISABLE;
    CarCAN->Init.NominalPrescaler = 16;
    CarCAN->Init.NominalSyncJumpWidth = 1;
    CarCAN->Init.NominalTimeSeg1 = 1;
    CarCAN->Init.NominalTimeSeg2 = 1;
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

can_status_t CarCAN_Recieve(BPS_Status_t *status, uint32_t id, uint8_t *data, TickType_t delay_ticks)
{

    can_status_t result = can_fd_recv(CarCAN, id, &carCAN_rx_header, data, delay_ticks);
    if (result == CAN_EMPTY)
    {
        return CAN_EMPTY;
    }
    if (result != CAN_OK)
    {
        return CAN_ERR;
    }

    if (carCAN_rx_header.DataLength < FDCAN_DLC_BYTES_5)
    {
        return CAN_ERR;
    }

    //TODO: implement unpacking function based on carCAN DBC




}
