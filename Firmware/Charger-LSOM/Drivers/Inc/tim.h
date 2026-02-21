

#pragma once

#include "stm32g4xx_hal.h"
#include "pinDef.h"
#include "common.h"

extern TIM_HandleTypeDef htim2;

extern TIM_HandleTypeDef htim5;

void MX_TIM2_Init(void);
void MX_TIM5_Init(void);

/**
 * @brief   HAL_Tim init callback function for post-initialization configuration of TIM peripherals. This function is called by the HAL library after the basic timer initialization is complete. It can be used to configure additional settings such as GPIO pins for PWM output, interrupts, or other timer features that require further setup beyond the initial configuration.
 * @param   htim Pointer to the TIM handle structure that contains the configuration information for the
 * @return  none
 */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
