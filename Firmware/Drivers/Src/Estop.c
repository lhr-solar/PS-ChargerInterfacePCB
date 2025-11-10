#include "stm32xx_hal.h"
#include "pinDef.h"
#include "Estop.h"


void Estop_Init(void)
{
    // Configure the GPIO pin as output push-pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESTOP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ESTOP_PORT, &GPIO_InitStruct);

}



void Estop_Read(bool* estop_state)
{
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN);

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