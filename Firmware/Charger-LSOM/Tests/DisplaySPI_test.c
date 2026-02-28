/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** DisplaySPI_test.c
 * Description: Test file for display driver. Tests all display functions and patterns.
 * Hardware: tests should be run with an active display connected to the SPI output pins. Check SPI pins with logic analyzer
 * The heartbeat LED should also be observed to ensure the system is still responsive while the display is active.
 * Observe Charge LED to ensure it toggles when drawing strings (indicates CPU is not blocked while waiting for display)
 * Test pattern should show a checkerboard pattern, then clear, then show "lil shah" on the display.
 * After the test pattern, the display should show "big dick bee"
 */

#include "DisplaySPI.h"
#include "spi.h"
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "StatusLED.h"
#include "gpio.h"
#include "stm32xx_hal.h"

TaskHandle_t DisplayTestTask_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;
TaskHandle_t InitTask_Handle = NULL;

StaticTask_t DisplayTestTask_Buffer;
StackType_t DisplayTestTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t InitTask_Buffer;
StackType_t InitTaskStack[configMINIMAL_STACK_SIZE];

// test pattern: checkerboard pattern (verifies SPI + all control pins)
// renders text on all 4 lines
// change contrast from min to max
// turning display on and off
void DisplayTestTask(void *argument)
{

    Display_TestPattern();
    vTaskDelay(pdMS_TO_TICKS(2000));

    Display_Clear();
    vTaskDelay(pdMS_TO_TICKS(500));

    Display_DrawString(0, 0, "big dick bee");
    Display_Update();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    MX_GPIO_Init();
    MX_SPI3_Init();
    Display_Init();

    vTaskDelete(NULL);
}

int main(void)
{

    DisplayTestTask_Handle = xTaskCreateStatic(
        DisplayTestTask,
        "Display Test",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        DisplayTestTaskStack,
        &DisplayTestTask_Buffer);

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
        "Init Tasks",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 3,
        InitTaskStack,
        &InitTask_Buffer);

    vTaskStartScheduler();

    while (1)
    {
    }

    return 0;
}
