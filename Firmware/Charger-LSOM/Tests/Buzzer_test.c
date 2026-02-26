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

TaskHandle_t BuzzerTask_Charging_Handle = NULL;
TaskHandle_t BuzzerTask_Alarm_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;

StaticTask_t BuzzerTask_Charging_Buffer;
StackType_t BuzzerTask_ChargingStack[configMINIMAL_STACK_SIZE];

StaticTask_t BuzzerTask_Alarm_Buffer;
StackType_t BuzzerTask_AlarmStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];


void BuzzerTask_Charging(void *argument)
{


    Display_DrawString(0, 0, "lil shah");
    
    while (1)
    {
        ChargeStart();
        vTaskDelay(pdMS_TO_TICKS(2000));
        ChargeStop();

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

    Buzzer_Init();

    //Leaving out for now for clean testing
    /*
    BuzzerTask_Alarm_Handle = xTaskCreateStatic(
        BuzzerTask_Alarm,
        "Buzzer Task Alarm",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        BuzzerTask_AlarmStack,
        &BuzzerTask_Alarm_Buffer);
    */
    

    BuzzerTask_Charging_Handle = xTaskCreateStatic(
        BuzzerTask_Charging,
        "Buzzer Task Charging",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 3,
        BuzzerTask_ChargingStack,
        &BuzzerTask_Charging_Buffer);

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
