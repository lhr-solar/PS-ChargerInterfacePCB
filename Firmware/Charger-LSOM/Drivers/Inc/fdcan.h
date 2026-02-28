
#pragma once

#include "stm32xx_hal.h"
#include "pinDef.h"


extern FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan3;


void MX_FDCAN1_Init(void);
void MX_FDCAN3_Init(void);


