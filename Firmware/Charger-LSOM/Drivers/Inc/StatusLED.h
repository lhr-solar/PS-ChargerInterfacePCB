#ifndef STATUSLEDS_H
#define STATUSLEDS_H

#include "stm32xx_hal.h"
#include "pinDef.h"

extern bool buzzer_active;
extern bool s_alarm;



typedef enum {
    LED_EVSE = 0,
    LED_CHARGE,
    LED_HEART,
    LED_FAULT,
    LED_HV
} status_led_t;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} LEDPins;

static const LEDPins LEDMaps[] = {
    [LED_EVSE] = {
        .port = LED_EVSE_GPIO_Port,
        .pin  = LED_EVSE_Pin
    },
    [LED_CHARGE] = {
        .port = LED_Charge_GPIO_Port,
        .pin  = LED_Charge_Pin
    },
    [LED_HEART] = {
        .port = LED_Heart_GPIO_Port,
        .pin  = LED_Heart_Pin
    },
    [LED_FAULT] = {
        .port = LED_Fault_GPIO_Port,
        .pin  = LED_Fault_Pin
    },
    [LED_HV] = {
        .port = LED_HV_GPIO_Port,
        .pin  = LED_HV_Pin
    },
};




//Status LED Pin Init

void Status_Leds_Init(void);

// utilized to turn LED on/off
void Status_Leds_Write(status_led_t led, bool state);


// utilized to toggle LED states such as flashing
void Status_Leds_Toggle(status_led_t led);

//turn all LEDs on for validation
void Status_Leds_All_On(void);


#endif
