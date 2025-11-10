#ifndef STATUSLEDS_H
#define STATUSLEDS_H

#include "stm32xx_hal.h"
#include "pinDef.h"

/*

#define CHARGE_FAULT_LED_PORT GPIOA
#define CHARGE_FAULT_LED_PIN GPIO_PIN_15

#define HEARTBEAT_SENSE_LED_PORT GPIOC
#define HEARTBEAT_SENSE_LED_PIN GPIO_PIN_10

#define BPS_SENSE_LED_PORT GPIOC
#define BPS_SENSE_LED_PIN GPIO_PIN_11

#define ELCON_SENSE_LED_PORT GPIOC
#define ELCON_SENSE_LED_PIN GPIO_PIN_12

*/

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