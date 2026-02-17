#include "StatusLED.h"
#include "pinDef.h"
#include "stm32g4xx_hal.h"
#include "gpio.h"

//read HV = HV active or not


//Fault = fault indicator

//Heart = power for 14V/24V

void HeartBeat(void){

    while (1) {


        HAL_GPIO_WritePin(LEDMaps[LED_HEART].port, LEDMaps[LED_HEART].pin, GPIO_PIN_SET);
        vTaskDelay(pdMS_TO_TICKS(750));

        HAL_GPIO_WritePin(LEDMaps[LED_HEART].port, LEDMaps[LED_HEART].pin, GPIO_PIN_RESET);
        vTaskDelay(pdMS_TO_TICKS(750));
    }

}



//Charge = charging status

//EVSE = evse detection





