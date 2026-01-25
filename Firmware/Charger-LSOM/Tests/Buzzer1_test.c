#include "Buzzer1.h"
#include "tim.h"
#include <stdlib.h>
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"


StaticTask_t initTaskBuffer;
StackType_t initTaskStack[configMINIMAL_STACK_SIZE];
StaticTask_t BuzzerTask_Charging_Buffer;
StackType_t BuzzerTask_ChargingStack[configMINIMAL_STACK_SIZE];
StaticTask_t BuzzerTask_Alarm_Buffer;
StackType_t BuzzerTask_AlarmStack[configMINIMAL_STACK_SIZE];



void BuzzerTask_Charging(void* argument){
    
    while(1){
        ChargeStart();
        vTaskDelay(pdMS_TO_TICKS(2000));
        ChargeStop();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void BuzzerTask_Alarm(void* argument){
    

    while(1){
        Alarm();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


int main (void) {

    SystemClock_Config();
    HAL_Init();
    MX_TIM5_Init();
    Buzzer_Init();

     xTaskCreateStatic(BuzzerTask_Alarm,
        "Buzzer Task Alarm",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        BuzzerTask_AlarmStack,
        &BuzzerTask_Alarm_Buffer);

    xTaskCreateStatic(BuzzerTask_Charging,
        "Buzzer Task Charging",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 5,
        BuzzerTask_ChargingStack,
        &BuzzerTask_Charging_Buffer);

    vTaskStartScheduler();

    while (1){


    }

    return 0; 

}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}