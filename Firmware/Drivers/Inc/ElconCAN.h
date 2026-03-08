#pragma once

#include <stdint.h>
#include "stm32xx_hal.h"
#include "pinDef.h"
#include "CAN_FD.h"

//TODO: add comments for each of the written functions

#define ELCONCAN_TX_ID  0x1806E5F4
#define ELCONCAN_RX_ID  0x18FF50E5

extern FDCAN_HandleTypeDef *ElconCAN;

typedef struct {
    float   output_voltage;
    float   output_current;
    uint8_t flag_hw_failure;
    uint8_t flag_over_temp;
    uint8_t flag_input_voltage_wrong;
    uint8_t flag_starting_state;
    uint8_t flag_comm_timeout;
} ElconStatus_t;



can_status_t ElconCAN_Init(void);
can_status_t ElconCAN_Send(float voltage_v, float current_a, uint8_t stop, TickType_t delay_ticks);
can_status_t ElconCAN_Recieve(ElconStatus_t* status, uint32_t id, uint8_t *data, TickType_t delay_ticks);
