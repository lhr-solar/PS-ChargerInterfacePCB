#ifndef STATUSLEDS_H
#define STATUSLEDS_H

#include "stm32xx_hal.h"
#include "pinDef.h"


typedef enum {
    CHARGE_FAULT_LED = 0,
    HEARTBEAT_SENSE_LED,
    BPS_SENSE_LED,
    ELCON_SENSE_LED,
    NUM_STATUS_LED
} status_led_t;

//Status LED Pin Init

void Status_Leds_Init(void);

// utilized to turn LED on/off
void Status_Leds_Write(status_led_t led, bool state);


// utilized to toggle LED states such as flashing
void Status_Leds_Toggle(status_led_t led);

//turn all LEDs on for validation
void Status_Leds_All_On(void);


#endif