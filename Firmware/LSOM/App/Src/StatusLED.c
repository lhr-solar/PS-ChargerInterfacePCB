#include "StatusLED.h"
#include "pinDef.h"



void Status_Leds_Init(void){


    
      // Initialize GPIO pins on GPIO A for Status LEDs
    GPIO_InitTypeDef led_a_init = {
      led_a_init.Mode = GPIO_MODE_OUTPUT_PP,
      led_a_init.Pull = GPIO_NOPULL,
      led_a_init.Pin = (CHARGE_FAULT_LED_PIN)
    };

    // Initialize GPIO pins on GPIO C for Status LEDs
    GPIO_InitTypeDef led_c_init = {
        led_c_init.Mode = GPIO_MODE_OUTPUT_PP,
        led_c_init.Pull = GPIO_NOPULL,
        led_c_init.Pin = (BPS_SENSE_LED_PIN | HEARTBEAT_SENSE_LED_PIN | ELCON_SENSE_LED_PIN)
    };

    __HAL_RCC_GPIOA_CLK_ENABLE();
    HAL_GPIO_Init(GPIOA, &led_a_init);

      __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_Init(GPIOC, &led_c_init);
}

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} LEDPins;

static const LEDPins LEDMaps[NUM_STATUS_LED] = {
    [CHARGE_FAULT_LED] = = {CHARGE_FAULT_LED_PORT, CHARGE_FAULT_LED_PIN},  
    [HEARTBEAT_SENSE_LED] = {HEARTBEAT_SENSE_LED_PORT, HEARTBEAT_SENSE_LED_PIN},
    [BPS_SENSE_LED] = {BPS_SENSE_LED_PORT, BPS_SENSE_LED_PIN},
    [ELCON_SENSE_LED] = {ELCON_SENSE_LED_PORT, ELCON_SENSE_LED_PIN},
}

void Status_Leds_Write(status_led_t led, bool state){


    if (led < NUM_STATUS_LED) {
        HAL_GPIO_WritePin(LEDMaps[led].port, LEDMaps[led].pin, state);
    }
    else {
        return; //out of total LED range
    }
}

void Status_Leds_Toggle(status_led_t led){
    if (led < NUM_STATUS_LED) {
        HAL_GPIO_TogglePin(LEDMaps[led].port, LEDMaps[led].pin);
    }
    else {
        return; //out of total LED range
    }
}

void Status_Leds_All_On(void) {
    for (int i = 0; i < NUM_STATUS_LED; i++) {
        Status_Leds_Write(i, true);
    }
    else {
        return; //out of total LED range
    }
}