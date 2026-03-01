/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** Estop.c
 * Wrapper that holds Estop functionality
 */

#include "stm32xx_hal.h"
#include "pinDef.h"
#include "Estop.h"
#include <stdio.h>
#include "DisplaySPI.h"
#include "StatusLED.h"

Estop_status_t Estop_State(void)
{

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN);

    // Active Low
    if (pin_state == GPIO_PIN_RESET)
    {
        printf("EStop Pressed!\n");
        Display_DrawString(0, 0, "EStop Pressed!@!");
        LED_State_t leds = {
            .evse_present = false,
            .charging = false,
            .fault = true,
            .hv_active = false,
        };
        LEDSet(&leds);

        return ESTOP_PRESSED;
    }

    if (pin_state == GPIO_PIN_SET)
    {

        printf("EStop NOT PRESSED, THUS RELEASED!\n");
        Display_DrawString(0, 0, "Estop Released!@@@!");


        LED_State_t leds = {
            .evse_present = false,
            .charging = false,
            .fault = false,
            .hv_active = false,
        };
        LEDSet(&leds);

        return ESTOP_RELEASED;
    }
    return ESTOP_RELEASED;
}
