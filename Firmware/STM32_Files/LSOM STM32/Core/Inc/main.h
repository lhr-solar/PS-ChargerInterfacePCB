/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MCU_PP_Pin GPIO_PIN_0
#define MCU_PP_GPIO_Port GPIOC
#define Display_RES_Pin GPIO_PIN_1
#define Display_RES_GPIO_Port GPIOC
#define MCU_CP_Pin GPIO_PIN_2
#define MCU_CP_GPIO_Port GPIOC
#define LED_EVSE_Pin GPIO_PIN_0
#define LED_EVSE_GPIO_Port GPIOA
#define LED_Charge_Pin GPIO_PIN_1
#define LED_Charge_GPIO_Port GPIOA
#define LED_Heart_Pin GPIO_PIN_2
#define LED_Heart_GPIO_Port GPIOA
#define LED_Fault_Pin GPIO_PIN_0
#define LED_Fault_GPIO_Port GPIOB
#define LED_HV_Pin GPIO_PIN_1
#define LED_HV_GPIO_Port GPIOB
#define Estop_Pin GPIO_PIN_13
#define Estop_GPIO_Port GPIOB
#define ElconCAN_RX_Pin GPIO_PIN_11
#define ElconCAN_RX_GPIO_Port GPIOA
#define ElconCAN_TX_Pin GPIO_PIN_12
#define ElconCAN_TX_GPIO_Port GPIOA
#define Display_NSS_Pin GPIO_PIN_15
#define Display_NSS_GPIO_Port GPIOA
#define Display_SCK_Pin GPIO_PIN_10
#define Display_SCK_GPIO_Port GPIOC
#define Display_MISO_Pin GPIO_PIN_11
#define Display_MISO_GPIO_Port GPIOC
#define Display_MOSI_Pin GPIO_PIN_12
#define Display_MOSI_GPIO_Port GPIOC
#define CarCAN_RX_Pin GPIO_PIN_3
#define CarCAN_RX_GPIO_Port GPIOB
#define CarCAN_TX_Pin GPIO_PIN_4
#define CarCAN_TX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
