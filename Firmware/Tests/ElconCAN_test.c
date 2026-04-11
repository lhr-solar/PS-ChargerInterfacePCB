/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** ElconCAN_test.c
 * Description: Test file for communicating with the Elcon Charger
 * Hardware: tests should be run with the Elcon plugged in from its LV output and its HV output connected to an battery pack
 * The heartbeat LED should also be observed to ensure the system is still responsive while the system is active and the HV and Charge LED will be continously active while CAN messages are being communicated
 */

#include "Buzzer.h"
#include "tim.h"
#include <stdbool.h>
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

// wrapper for packing and sending a charge command to the Elcon charger (CAN ID 0x1806E5F4)
static can_status_t Elcon_SendChargeCommand(float voltage_v, float current_a, bool stop, TickType_t delay_ticks)
{
    if (voltage_v < 0.0f || voltage_v > 6553.5f ||
        current_a < 0.0f || current_a > 6553.5f)
    {
        return CAN_ERR;
    }

    uint16_t v_scaled = (uint16_t)(voltage_v * 10.0f);
    uint16_t c_scaled = (uint16_t)(current_a * 10.0f);

    uint8_t tx_data[8] = {0};
    tx_data[0] = (uint8_t)(v_scaled >> 8);   // voltage high byte
    tx_data[1] = (uint8_t)(v_scaled & 0xFF); // voltage low byte
    tx_data[2] = (uint8_t)(c_scaled >> 8);   // current high byte
    tx_data[3] = (uint8_t)(c_scaled & 0xFF); // current low byte
    tx_data[4] = (uint8_t)stop;              // stop flag (bytes 5-7 reserved, zero)

    return ElconCAN_Send(ELCONCAN_TX_ID, tx_data, FDCAN_DLC_BYTES_5, delay_ticks);
}

void ElconCAN_Task(void *argument)
{
    ElconStatus_t status = {0};
    uint8_t rx_data[8] = {0};
    char buf[32];

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(500);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // output: 04 B0 00 32 00 00 00 00 (120V, 5A)
        Elcon_SendChargeCommand(120, 5, 0, portMAX_DELAY);

        can_status_t recv_result = ElconCAN_Receive(&status, ELCONCAN_RX_ID, rx_data, 0);

        Display_Clear();

        if (recv_result == CAN_OK)
        {
            snprintf(buf, sizeof(buf), "V:%.1fV I:%.1fA",
                     status.output_voltage_dv / 10.0f, status.output_current_da / 10.0f);
            Display_DrawString(0, 0, buf);

            snprintf(buf, sizeof(buf), "HW:%d OT:%d IV:%d",
                     status.flag_hw_failure, status.flag_over_temp, status.flag_input_voltage_wrong);
            Display_DrawString(0, 8, buf);

            snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X%02X%02X",
                     rx_data[0], rx_data[1], rx_data[2], rx_data[3],
                     rx_data[4], rx_data[5], rx_data[6], rx_data[7]);
            Display_DrawString(0, 16, buf);

            LED_State_t leds = {
                .evse_present = false,
                .charging = true,
                .fault = false,
                .hv_active = false,
            };
            LEDSet(&leds);
        }
        else if (recv_result == CAN_ERR)
        {
            Display_DrawString(0, 0, "CAN ERR");

            snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X%02X%02X",
                     rx_data[0], rx_data[1], rx_data[2], rx_data[3],
                     rx_data[4], rx_data[5], rx_data[6], rx_data[7]);
            Display_DrawString(0, 8, buf);

            LED_State_t leds = {
                .evse_present = false,
                .charging = false,
                .fault = true,
                .hv_active = false,
            };
            LEDSet(&leds);
        }
        else // CAN_EMPTY
        {
            Display_DrawString(0, 0, "Waiting for Elcon");

            snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X%02X%02X",
                     rx_data[0], rx_data[1], rx_data[2], rx_data[3],
                     rx_data[4], rx_data[5], rx_data[6], rx_data[7]);
            Display_DrawString(0, 8, buf);

            LED_State_t leds = {
                .evse_present = false,
                .charging = false,
                .fault = false,
                .hv_active = true,
            };
            LEDSet(&leds);
        }
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
        ElconCAN_Task,
        "ElconCAN Tasks",
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
