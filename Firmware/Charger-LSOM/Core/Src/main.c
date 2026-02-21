/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** main.c
 * Main application file for the PS-ChargerInterfacePCB firmware.
 * balls
 */

#include "tim.h"
#include "buzzer.h"
#include "common.h"

#include "stm32g4xx_hal.h"
#include "gpio.h"

/**
 * @brief  The application entry point.
 * @retval int
 */

int main(void)
{

  // peripheral inits
  HAL_Init();

  SystemClock_Config();

  MX_TIM5_Init();

  while (1)
  {
  }
}


