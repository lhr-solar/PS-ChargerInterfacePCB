#include "StatusLED.h"
#include "pinDef.h"
#include "stm32g4xx_hal.h"
#include "gpio.h"
#include "Estop.h"

/**
 * @brief   Indicates the presence of a high voltage condition by controlling the corresponding LED.
 * @param   hv_active true if a high voltage condition is present, false otherwise
 * @return  Sets the high voltage LED on if hv_active is true, off if false.
 */

void HV_Indicator(bool hv_active)
{
    if (hv_active)
    {

        HAL_GPIO_WritePin(LEDMaps[LED_HV].port, LEDMaps[LED_HV].pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LEDMaps[LED_HV].port, LEDMaps[LED_HV].pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief   Indicates the presence of a fault condition by controlling the corresponding LED.
 * @param   fault true if a fault condition is present, false otherwise
 * @return  Toggles the fault LED on and off at a regular interval (250ms on, 250ms off) to create a blinking effect.
 */

void Fault_Indicator(bool fault)
{
    if (fault)
    {
        HAL_GPIO_WritePin(LEDMaps[LED_FAULT].port, LEDMaps[LED_FAULT].pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(200));
        HAL_GPIO_WritePin(LEDMaps[LED_FAULT].port, LEDMaps[LED_FAULT].pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    else
    {
        HAL_GPIO_WritePin(LEDMaps[LED_FAULT].port, LEDMaps[LED_FAULT].pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief   Consistent Heartbeat LED to indicate that the system is alive and responsive.
 * @return  Toggles the heartbeat LED on and off at a regular interval (750ms on, 750ms off) to create a blinking effect.
 */
void HeartBeat(void)
{

    while (1)
    {

        HAL_GPIO_WritePin(LEDMaps[LED_HEART].port, LEDMaps[LED_HEART].pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(500));

        HAL_GPIO_WritePin(LEDMaps[LED_HEART].port, LEDMaps[LED_HEART].pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
 * @brief   Indicates the charging status by controlling the corresponding LED.
 * @param   charging true if the system is currently charging, false otherwise
 * @return  Toggles the charging LED on and off at a regular interval (1000ms on, 1000ms off) to create a blinking effect when charging is true. Turns off the LED when charging is false.
 */
void Charging_Indicator(bool charging)
{
    while (charging)
    {
        HAL_GPIO_WritePin(LEDMaps[LED_CHARGE].port, LEDMaps[LED_CHARGE].pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(1000));
        HAL_GPIO_WritePin(LEDMaps[LED_CHARGE].port, LEDMaps[LED_CHARGE].pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    HAL_GPIO_WritePin(LEDMaps[LED_CHARGE].port, LEDMaps[LED_CHARGE].pin, GPIO_PIN_RESET);
}

/**
 * @brief   Indicates the presence of an EVSE by controlling the corresponding LED.
 * @param   evse_present true if EVSE is present, false otherwise
 * @return  Sets the EVSE LED on if evse_present is true, off if false.
 */
void EVSE_Indicator(bool evse_present)
{

    if (evse_present)
    {

        HAL_GPIO_WritePin(LEDMaps[LED_EVSE].port, LEDMaps[LED_EVSE].pin, GPIO_PIN_SET);
    }
    else
    {
        HAL_GPIO_WritePin(LEDMaps[LED_EVSE].port, LEDMaps[LED_EVSE].pin, GPIO_PIN_RESET);
    }
}
