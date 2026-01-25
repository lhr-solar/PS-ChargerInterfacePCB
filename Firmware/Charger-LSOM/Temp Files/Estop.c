#include "stm32xx_hal.h"
#include "pinDef.h"
#include "Estop.h"


void EStop_Init(void)
{
    // Configure the GPIO pin as output push-pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Estop_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(Estop_GPIO_Port, &GPIO_InitStruct);

}



void Estop_Read(bool* estop_state)
{
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(Estop_GPIO_Port, Estop_Pin);

    //Active Low
    if (pin_state == GPIO_PIN_RESET){
        *estop_state = true; //EStop Pressed
    }
    else {
        *estop_state = false; //EStop Not Pressed
    }
}

//if true, send CAN msg to indicate Estop pressed to BPS or CarCAN
//if false, normal state