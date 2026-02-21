#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"

typedef enum {
    ESTOP_PRESSED,
    ESTOP_RELEASED
} Estop_status_t;

/**
 * @brief   Reads the Estop state
 * @param   estop_state Pointer to a boolean variable that will be set to true if the Estop is pressed, false otherwise
 * @return  true or false depending on the state of the Estop pin. This function reads the GPIO pin connected to the Estop button and updates the provided boolean variable with the current state (active low).
 */
Estop_status_t Estop_State();
