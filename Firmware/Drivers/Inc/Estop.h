#ifndef Estop_H
#define Estop_H

#include "stm32xx_hal.h"
#include "pinDef.h"


void Estop_Init(void);

//True = EStop Pressed
//False = default not pressed state
void Estop_Read(bool* estop_state);

#endif