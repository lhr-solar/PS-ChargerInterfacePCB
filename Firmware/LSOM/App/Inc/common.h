#ifndef COMMON_H
#define COMMON_H

#include "stm32xx_hal.h"
// #include "CAN.h"

#define RATE_HEARTBEAT_MS   500
#define RATE_LED_MS         100
#define RATE_CAN_TX_MS      100

#define FAULT_MESSAGE_DELAY pdMS_TO_TICKS(200)

extern uint32_t fault_bitmap;

typedef enum {
    FAULT_NONE = 0,

    FAULT_ESTOP = 1 << 1,

    FAULT_ELCON_UV = 1 << 2,
    FAULT_ELCON_OV = 1 << 3,
    FAULT_ELCON_OC = 1 << 4,

    FAULT_BPS_OV = 1 << 8,
    FAULT_BPS_OC = 1 << 9,

    FAULT_DISPLAY = 1 << 10,
    FAULT_BUZZER = 1 << 11,

    FAULT_HEARTBEAT_MISSED = 1 << 12
} fault_state_t;


#endif