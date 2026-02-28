/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** Estop_test.c
 * Description: Test file for Estop driver. Tests all Estop functions and its PWM patternsp.
 * Hardware: tests should be run with an active Estop connected to the Estop output pin and buzzer lowk will make
 * The heartbeat LED should also be observed to ensure the system is still responsive while the Estop is active.
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
#include "spi.h"

TaskHandle_t Estop_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;
TaskHandle_t FaultLEDTask_Handle = NULL;
TaskHandle_t InitTask_Handle = NULL;

StaticTask_t Estop_Buffer;
StackType_t EstopStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t FaultLEDTask_Buffer;
StackType_t FaultLEDTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t InitTask_Buffer;
StackType_t InitTaskStack[configMINIMAL_STACK_SIZE];

void EstopTask(void *argument)
{
    vTaskDelay(pdMS_TO_TICKS(200));
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

        if (HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN) == GPIO_PIN_RESET)
        {
            LED_State_t leds = {
            .evse_present = false,
            .charging = false,
            .fault = true,
            .hv_active = false,
        };
        LEDSet(&leds);
        }
        else
        {
            LED_State_t leds = {
                .evse_present = false,
                .charging = false,
                .fault = false,
                .hv_active = false,
            };
            LEDSet(&leds);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void HeartBeatTask(void *argument)
{
    HeartBeat();
}

void InitTask(void *argument)
{
    HAL_Init();
    SystemClock_Config();
    MX_TIM5_Init();
    MX_SPI3_Init();
    MX_GPIO_Init();

    vTaskDelete(NULL);
}

int main(void)
{

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
        tskIDLE_PRIORITY + 3,
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

    InitTask_Handle = xTaskCreateStatic(
        InitTask,
        "Init Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 4,
        InitTaskStack,
        &InitTask_Buffer);

    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}
