/* Copyright (c) 2018-2022 UT Longhorn Racing Solar */
/** main.c
 * Main application file for the PS-ChargerInterfacePCB firmware.
 */

#include "main.h"
#include "tim.h"
#include "buzzer.h"




/**
  * @brief  The application entry point.
  * @retval int
  */
 
int main(void)
{

  //peripheral inits
  HAL_Init();

  SystemClock_Config();

  MX_TIM5_Init();


  while (1)
  {
    
  }
  
}



void Error_Handler(void)
{

  __disable_irq();
  while (1)
  {
  }
  
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
