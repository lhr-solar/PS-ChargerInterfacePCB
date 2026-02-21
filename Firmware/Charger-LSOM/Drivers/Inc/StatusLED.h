#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"


extern bool is_fault_active;


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

// Status LED Pin Init


/**
 * @brief   Heartbeat LED to showcase RTOS is working
 * @param   None
 * @return  None
 */
void HeartBeat(void);

void Fault_Indicator(bool fault);

void Charging_Indicator(bool charging);

void HV_Indicator(bool hv_active);

void EVSE_Indicator(bool evse_present);
