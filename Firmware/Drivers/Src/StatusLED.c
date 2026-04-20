#include "StatusLED.h"
#include "pinDef.h"
#include "stm32xx_hal.h"
#include "gpio.h"
#include "Estop.h"


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



void LEDSet(const LED_State_t *state)
{
    HAL_GPIO_WritePin(LEDMaps[LED_EVSE].port,   LEDMaps[LED_EVSE].pin,   state->evse_present ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEDMaps[LED_CHARGE].port,  LEDMaps[LED_CHARGE].pin,  state->charging     ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEDMaps[LED_FAULT].port,   LEDMaps[LED_FAULT].pin,   state->fault        ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LEDMaps[LED_HV].port,      LEDMaps[LED_HV].pin,      state->hv_active    ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
