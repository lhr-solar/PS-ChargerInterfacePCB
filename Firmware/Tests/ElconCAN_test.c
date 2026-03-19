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
#include "CAN_FD.h"

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
    Display_DrawString(0, 0, "i think its sending");

    ElconStatus_t status;
    uint8_t rx_data[8];
    char buf[32];

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Flush any stale RX messages that arrived since last loop
        while (ElconCAN_Recieve(&status, ELCONCAN_RX_ID, rx_data, 0) == CAN_OK) {}

        // output: 01 F4 00 14 00 00 00 00 (50V, 2A)
        ElconCAN_Send(50, 2, 0, portMAX_DELAY);

        // Wait for fresh response — 400ms gives plenty of margin within the 500ms period
        if (ElconCAN_Recieve(&status, ELCONCAN_RX_ID, rx_data, pdMS_TO_TICKS(400)) == CAN_OK)
        {
            snprintf(buf, sizeof(buf), "V:%.1fV  I:%.1fA",
                     status.output_voltage, status.output_current);
            Display_DrawString(0, 8, buf);

            snprintf(buf, sizeof(buf), "HW:%d OT:%d IV:%d",
                     status.flag_hw_failure, status.flag_over_temp, status.flag_input_voltage_wrong);
            Display_DrawString(0, 16, buf);

            HAL_GPIO_WritePin(LED_CHARGE_PORT, LED_CHARGE_PIN, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_FAULT_PORT,  LED_FAULT_PIN,  GPIO_PIN_RESET);
        }
        else
        {
            Display_Clear();
            Display_DrawString(0, 10, "RX is on monkey balls");

            HAL_GPIO_WritePin(LED_CHARGE_PORT, LED_CHARGE_PIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_FAULT_PORT,  LED_FAULT_PIN,  GPIO_PIN_SET);
        }

        HAL_GPIO_WritePin(LED_HV_PORT, LED_HV_PIN, GPIO_PIN_SET);
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
