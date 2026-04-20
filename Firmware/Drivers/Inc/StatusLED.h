#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"


#define HEARTBEAT_PERIOD 750 // 750 ms heartbeat period, can be adjusted as needed

typedef enum
{
    LED_EVSE = 0, // ON when the EVSE handshake signal is present (AC supply connected and negotiated)
    LED_CHARGE,   // ON when actively charging and communicating with BPS (BPS_Charge_OK = 1)
    LED_HEART,    // Heartbeat — blinks at HEARTBEAT_PERIOD to indicate the RTOS scheduler is running
    LED_FAULT,    // ON when any fault is active (BPS denial, Elcon fault, CAN timeout, etc.)
    LED_HV        // ON when HV is present at the Elcon output (charger is energized on the HV side)
} status_led_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} LED_pin_t;

typedef struct
{
    bool evse_present; // EVSE handshake signal present
    bool charging;     // actively charging with BPS permission
    bool fault;        // any fault is active
    bool hv_active;    // HV present at Elcon output
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
