#pragma once

#include <stdint.h>
#include "stm32xx_hal.h"
#include "pinDef.h"
#include "CAN_FD.h"

//TODO: update this based on the CarCAN DBC files
typedef struct {
    float   output_voltage;
    float   output_current;
    uint8_t flag_hw_failure;
    uint8_t flag_over_temp;
    uint8_t flag_input_voltage_wrong;
    uint8_t flag_starting_state;
    uint8_t flag_comm_timeout;
} ElconStatus_t;

can_status_t CarCAN_Init(void);

//TODO: update both functions with input params
can_status_t CarCAN_Send(uint32_t id, uint8_t data[8], TickType_t delay_ticks);

can_status_t CarCAN_Recieve(BPS_Status_t* status, uint32_t id, uint8_t *data, TickType_t delay_ticks);
