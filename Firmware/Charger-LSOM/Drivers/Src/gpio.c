
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
  HAL_GPIO_WritePin(Display_RES_GPIO_Port, Display_RES_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_EVSE_Pin|LED_Charge_Pin|LED_Heart_Pin|Display_NSS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_Fault_Pin|LED_HV_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : MCU_PP_Pin MCU_CP_Pin */
  GPIO_InitStruct.Pin = MCU_PP_Pin|MCU_CP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Display_RES_Pin */
  GPIO_InitStruct.Pin = Display_RES_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Display_RES_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_EVSE_Pin LED_Charge_Pin LED_Heart_Pin DIsplay_NSS_Pin */
  GPIO_InitStruct.Pin = LED_EVSE_Pin|LED_Charge_Pin|LED_Heart_Pin|Display_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Fault_Pin LED_HV_Pin */
  GPIO_InitStruct.Pin = LED_Fault_Pin|LED_HV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Estop_Pin */
  GPIO_InitStruct.Pin = Estop_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Estop_GPIO_Port, &GPIO_InitStruct);

}
/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
