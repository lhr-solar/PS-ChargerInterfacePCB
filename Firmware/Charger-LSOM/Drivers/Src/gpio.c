
#include "gpio.h"
#include "pinDef.h"
#include "stm32g4xx_hal.h"

void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DISPLAY_RES_PORT, DISPLAY_RES_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_EVSE_PIN | LED_CHARGE_PIN | LED_HEART_PIN | DISPLAY_NSS_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_FAULT_PIN | LED_HV_PIN, GPIO_PIN_RESET);

  /*Configure GPIO pins : MCU_PP_Pin MCU_CP_Pin */
  GPIO_InitStruct.Pin = MCU_PP_PIN | MCU_CP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Display_RES_Pin */
  GPIO_InitStruct.Pin = DISPLAY_RES_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DISPLAY_RES_PORT, &GPIO_InitStruct);
  
  /*Configure GPIO pins : LED_EVSE_Pin LED_Charge_Pin LED_Heart_Pin DIsplay_NSS_Pin */
  GPIO_InitStruct.Pin = LED_EVSE_PIN | LED_CHARGE_PIN | LED_HEART_PIN | DISPLAY_NSS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Fault_Pin LED_HV_Pin */
  GPIO_InitStruct.Pin = LED_FAULT_PIN | LED_HV_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Estop_Pin */
  GPIO_InitStruct.Pin = ESTOP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ESTOP_PORT, &GPIO_InitStruct);
}