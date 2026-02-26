#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"


typedef enum
{
    LED_EVSE = 0,
    LED_CHARGE,
    LED_HEART,
    LED_FAULT,
    LED_HV
} status_led_t;


typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} LED_pin_t;

static const LED_pin_t LEDMaps[] = {
    [LED_EVSE] = {.port = LED_EVSE_PORT, .pin = LED_EVSE_PIN},
    [LED_CHARGE] = {.port = LED_CHARGE_PORT, .pin = LED_CHARGE_PIN},
    [LED_HEART] = {.port = LED_HEART_PORT, .pin = LED_HEART_PIN},
    [LED_FAULT] = {.port = LED_FAULT_PORT, .pin = LED_FAULT_PIN},
    [LED_HV] = {.port = LED_HV_PORT, .pin = LED_HV_PIN},
};


/**
 * @brief   Heartbeat LED to showcase RTOS is working
 * @param   None
 * @return  None
 */
void HeartBeat(void);


/**
 * @brief   Fault Indicator LED to show any fault within the system (should never reach this state)
 * @param   fault true if a fault condition is present, false otherwise
 * @return  None
 */
void Fault_Indicator(bool fault);



/**
 * @brief   Charging Indicator LED to present when system is actively charging, usually off otherwise
 * @param   charging true if the system is currently charging, false otherwise
 * @return  None
 */

void Charging_Indicator(bool charging);

/**
 * @brief   HV Indicator LED to present when system is actively charging, usually off otherwise
 * @param   hv_active defaults to true if the system is currently charging, false otherwise
 * @return  None
 */

void HV_Indicator(bool hv_active);


/**
 * @brief   EVSE LED to present when the EVSE system is detected and ready to go, off otherwise
 * @param   evse_present true if EVSE is present, false otherwise
 * @return  None
 */
void EVSE_Indicator(bool evse_present);
