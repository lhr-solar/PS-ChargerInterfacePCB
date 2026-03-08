#pragma once

#include <stdint.h>
#include "stm32xx_hal.h"
#include "pinDef.h"

void CarCAN_Init(void);


//TODO: update both functions with input params
void CarCAN_Send(void);

void CarCAN_Recieve(void);
