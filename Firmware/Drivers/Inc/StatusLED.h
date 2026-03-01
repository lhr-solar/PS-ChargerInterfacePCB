#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"


#define HEARTBEAT_PERIOD 3000 // 3000 ms heartbeat period, can be adjusted as needed

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

typedef struct
{
    bool evse_present;
    bool charging;
    bool fault;
    bool hv_active;
} LED_State_t;

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
 * @brief   Sets all status LEDs at once using the LED_State_t struct via LEDMaps
 * @param   state Pointer to LED_State_t containing desired state for each LED
 * @return  None
 */

 /*
 Example code:
 LED_State_t leds = {
    .evse_present = true,
    .charging     = true,
    .fault        = false,
    .hv_active    = true,
};
LEDSet(&leds);
 
 */
void LEDSet(const LED_State_t *state);
