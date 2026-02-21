/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** Buzzer_test.c
 * Description: Test file for buzzer driver. Tests all buzzer functions and patterns.
 * Hardware: tests should be run with an active buzzer connected to the buzzer output pin
 * The heartbeat LED should also be observed to ensure the system is still responsive while the buzzer is active.
 */

#include "Buzzer.h"
#include "tim.h"
#include <stdlib.h>
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "common.h"
#include "StatusLED.h"
#include "DisplaySPI.h"
#include "Estop.h"
#include "gpio.h"

TaskHandle_t Estop_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;
TaskHandle_t FaultLEDTask_Handle = NULL;

StaticTask_t Estop_Buffer;
StackType_t EstopStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t FaultLEDTask_Buffer;
StackType_t FaultLEDTaskStack[configMINIMAL_STACK_SIZE];

void EstopTask(void *argument)
{

    Display_Init();
    Display_Clear();
    vTaskDelay(pdMS_TO_TICKS(200));

    while (1)
    {
        Estop_State();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



void FaultLEDTask(void *argument)
{
    while (1)
    {
        if (is_fault_active == true)
        {
            HAL_GPIO_TogglePin(LEDMaps[LED_FAULT].port, LEDMaps[LED_FAULT].pin);
            vTaskDelay(pdMS_TO_TICKS(250));
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

void HeartBeatTask(void *argument)
{
    HeartBeat();
}

int main(void)
{

    HAL_Init();
    SystemClock_Config();
    MX_TIM5_Init();

    MX_GPIO_Init();

    FaultLEDTask_Handle = xTaskCreateStatic(
        FaultLEDTask,
        "Fault LED Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        FaultLEDTaskStack,
        &FaultLEDTask_Buffer);


    Estop_Handle = xTaskCreateStatic(
        EstopTask,
        "Estop Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        EstopStack,
        &Estop_Buffer);

    HeartBeatTask_Handle = xTaskCreateStatic(
        HeartBeatTask,
        "Heartbeat LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        HeartBeatTaskStack,
        &HeartBeatTask_Buffer);

    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}
