/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** Buzzer_test.c
 * Description: Test file for buzzer driver. Tests all buzzer functions and patterns.
 * Hardware: tests should be run with an active buzzer connected to the buzzer output pin
 * The heartbeat LED should also be observed to ensure the system is still responsive while the buzzer is active.
 */

#include "Buzzer.h"
#include "tim.h"
#include <stdlib.h>
#include <stdio.h>
#include "gpio.h"
#include "spi.h"
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "common.h"
#include "StatusLED.h"
#include "DisplaySPI.h"
#include "ElconCAN.h"

TaskHandle_t ElconCAN_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;
TaskHandle_t InitTask_Handle = NULL;

StaticTask_t ElconCANTask_Buffer;
StackType_t ElconCANTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t InitTask_Buffer;
StackType_t InitTaskStack[configMINIMAL_STACK_SIZE];

void ElconCAN_TXTask(void *argument)
{
    Display_DrawString(0, 0, "trying to send shit");

    ElconStatus_t status;
    uint8_t rx_data[8];

    while (1)
    {

        // output 0B B8 00 C8 00 00 00 00
        ElconCAN_Send(300, 20, 0, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(2000));

        char buf[32];
        if (ElconCAN_Recieve(&status, ELCONCAN_RX_ID, rx_data, portMAX_DELAY) == CAN_OK)
        {
            printf(buf, sizeof(buf), "V:%.1f I:%.1f", status.output_voltage, status.output_current);
            Display_DrawString(0, 10, buf);

            printf(buf, sizeof(buf), "HW:%d OT:%d IV:%d",
                     status.flag_hw_failure, status.flag_over_temp, status.flag_input_voltage_wrong);
            Display_DrawString(0, 20, buf);
        }
        else
        {
            Display_DrawString(0, 10, "RX monkey balls");
        }

        HAL_GPIO_TogglePin(LED_HV_PORT, LED_HV_PIN);
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

    ElconCAN_Init();

    Display_Init();

    vTaskDelete(NULL);
}

int main(void)
{

    ElconCAN_Handle = xTaskCreateStatic(
        ElconCAN_TXTask,
        "ElconCAN tryna send",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        ElconCANTaskStack,
        &ElconCANTask_Buffer);

    HeartBeatTask_Handle = xTaskCreateStatic(
        HeartBeatTask,
        "Heartbeat LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        HeartBeatTaskStack,
        &HeartBeatTask_Buffer);

    InitTask_Handle = xTaskCreateStatic(
        InitTask,
        "Init Task",
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
