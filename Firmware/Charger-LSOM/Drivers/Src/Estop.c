#include "stm32xx_hal.h"
#include "pinDef.h"
#include "Estop.h"


void EStop_Init(void)
{
    // Configure the GPIO pin as output push-pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Estop_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(Estop_GPIO_Port, &GPIO_InitStruct);

}


bool Estop_State (void){

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(Estop_GPIO_Port, Estop_Pin);

    //Active Low
    if (pin_state == GPIO_PIN_SET){
        return true; //EStop Pressed
    }
    else {
        return false; //EStop Not Pressed
    }
}

//if true, send CAN msg to indicate Estop pressed to BPS or CarCAN
//if false, normal state