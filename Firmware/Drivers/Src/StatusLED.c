#include "StatusLED.h"
#include "pinDef.h"
#include "stm32xx_hal.h"
#include "gpio.h"
#include "Estop.h"

/**
 * @brief   Consistent Heartbeat LED to indicate that the system is alive and responsive.
 * @return  Toggles the heartbeat LED on and off at HEARTBEAT_PERIOD intervals.
 */
void HeartBeat(void)
{
    while (1)
    {
        HAL_GPIO_WritePin(LEDMaps[LED_HEART].port, LEDMaps[LED_HEART].pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_PERIOD));

        HAL_GPIO_WritePin(LEDMaps[LED_HEART].port, LEDMaps[LED_HEART].pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(HEARTBEAT_PERIOD));
    }
}

/**
 * @brief   Sets all status LEDs at once using the LED_State_t struct via LEDMaps.
 * @param   state Pointer to LED_State_t containing desired state for each LED.
 */
void LEDSet(const LED_State_t *state)
{
    HAL_GPIO_WritePin(LEDMaps[LED_EVSE].port,   LEDMaps[LED_EVSE].pin,   state->evse_present ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEDMaps[LED_CHARGE].port,  LEDMaps[LED_CHARGE].pin,  state->charging     ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEDMaps[LED_FAULT].port,   LEDMaps[LED_FAULT].pin,   state->fault        ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEDMaps[LED_HV].port,      LEDMaps[LED_HV].pin,      state->hv_active    ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
