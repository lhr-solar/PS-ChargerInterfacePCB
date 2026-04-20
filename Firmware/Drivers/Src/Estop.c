/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** Estop.c
 * Wrapper that holds Estop functionality
 */

#include "stm32xx_hal.h"
#include "pinDef.h"
#include "Estop.h"
Estop_status_t Estop_State(void)
{
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN);

    if (pin_state == GPIO_PIN_RESET)
    {
        return ESTOP_PRESSED;
    }

    if (pin_state == GPIO_PIN_SET)
    {
        return ESTOP_RELEASED;
    }

    return ESTOP_RELEASED;
}
