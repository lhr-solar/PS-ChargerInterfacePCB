/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** CarCAN_test.c
 * Description: Integration task for CarCAN + ElconCAN charging control.
 *              Charges via Elcon when BPS grants permission (BPS_Charge_OK=1).
 *              Stops immediately and faults on BPS denial or any Elcon fault flag.
 *              Propagates Elcon faults back to CarCAN as BPS_Charge_OK=0.
 * Hardware: Elcon connected on LV/HV. BPS broadcasting on CarCAN (FDCAN3).
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "Buzzer.h"
#include "tim.h"
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
#include "CarCAN.h"
#include "CAN_FD.h"

#define CHARGER_TASK_PERIOD_MS   500U
#define ELCON_TARGET_VOLTAGE_V   120.0f
#define ELCON_TARGET_CURRENT_A   25.0f
// BPS broadcasts at 10Hz (100ms). Fault after 3 missed frames.
#define BPS_STATUS_TIMEOUT_MS    300U
// Elcon broadcasts at ~1Hz (1000ms). Fault after 500ms silence.
#define ELCON_STATUS_TIMEOUT_MS  500U

TaskHandle_t ChargerTask_Handle    = NULL;
TaskHandle_t HeartBeatTask_Handle  = NULL;
TaskHandle_t InitTask_Handle       = NULL;

StaticTask_t ChargerTask_Buffer;
StackType_t  ChargerTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t  HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t InitTask_Buffer;
StackType_t  InitTaskStack[configMINIMAL_STACK_SIZE];

// Pack and send a charge command to the Elcon (CAN ID 0x1806E5F4)
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
    tx_data[0] = (uint8_t)(v_scaled >> 8);
    tx_data[1] = (uint8_t)(v_scaled & 0xFF);
    tx_data[2] = (uint8_t)(c_scaled >> 8);
    tx_data[3] = (uint8_t)(c_scaled & 0xFF);
    tx_data[4] = (uint8_t)stop;

    return ElconCAN_Send(ELCONCAN_TX_ID, tx_data, delay_ticks);
}

// Broadcast BPS_Charge_OK=0 on CarCAN to notify other nodes of a fault
static void CarCAN_BroadcastFault(void)
{
    uint8_t fault_payload[] = BPS_STATUS_FAULT_PAYLOAD;
    CarCAN_Send(BPS_Status_ID, fault_payload, portMAX_DELAY);
}

// Broadcast BPS_Charge_OK=1 on CarCAN to signal healthy charging
static void CarCAN_BroadcastOK(void)
{
    uint8_t ok_payload[] = BPS_STATUS_OK_PAYLOAD;
    CarCAN_Send(BPS_Status_ID, ok_payload, portMAX_DELAY);
}

void Charger_Task(void *argument)
{
    ElconStatus_t          elcon_status     = {0};
    CarCAN_BPS_Aggregate_t bps_agg          = {0};  //used later when implemented aggregate arr
    uint8_t  rx_data[8];
    uint32_t rx_id;
    bool     bps_charge_ok      = false;
    TickType_t xLastBPS_Tick    = xTaskGetTickCount();  
    TickType_t xLastElcon_Tick  = xTaskGetTickCount();  

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(CHARGER_TASK_PERIOD_MS);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        while (CarCAN_Receive(&rx_id, rx_data, 0) == CAN_OK)
        {
            if (rx_id == BPS_Status_ID)
            {
                bps_charge_ok  = (rx_data[BPS_STATUS_FLAGS_BYTE_IDX] & BPS_STATUS_CHARGE_OK_MASK) != 0;
                xLastBPS_Tick  = xTaskGetTickCount();
            }
            else if (rx_id == BPS_Aggregate_Arr_ID)
            {
                CarCAN_Unpack_BPS_Aggregate(rx_data, &bps_agg);
                // TODO: min/max cell voltage checks
            }
        }

        //bps timeout calculation
        if ((xTaskGetTickCount() - xLastBPS_Tick) > pdMS_TO_TICKS(BPS_STATUS_TIMEOUT_MS))
        {
            Elcon_SendChargeCommand(0.0f, 0.0f, true, portMAX_DELAY);
            CarCAN_BroadcastFault();
            faultBits_set(FAULT_CARCAN_HEARTBEAT_MISSED);
            LED_State_t leds = { .fault = true };
            LEDSet(&leds);
            continue;
        }

        // bps charging not okay
        if (!bps_charge_ok)
        {
            Elcon_SendChargeCommand(0.0f, 0.0f, true, portMAX_DELAY);
            faultBits_set(FAULT_BPS_CHARGE_NOT_OK);
            LED_State_t leds = { .fault = true };
            LEDSet(&leds);
            continue;
        }

        //charging allowed
        Elcon_SendChargeCommand(ELCON_TARGET_VOLTAGE_V, ELCON_TARGET_CURRENT_A, false, portMAX_DELAY);

        //read elcon incoming data
        can_status_t elcon_result = ElconCAN_Receive(&elcon_status, ELCONCAN_RX_ID, rx_data, 0);

        if (elcon_result == CAN_OK)
        {
            xLastElcon_Tick = xTaskGetTickCount();

            bool elcon_fault = elcon_status.flag_hw_failure
                            || elcon_status.flag_over_temp
                            || elcon_status.flag_input_voltage_wrong
                            || elcon_status.flag_starting_state
                            || elcon_status.flag_comm_timeout;

            //if any faults occur from Elcon side
            if (elcon_fault)
            {
                Elcon_SendChargeCommand(0.0f, 0.0f, true, portMAX_DELAY);
                CarCAN_BroadcastFault();
                faultBits_set(FAULT_ELCON_HARDWARE);
                LED_State_t leds = { .fault = true };
                LEDSet(&leds);
            }
            else
            {
                CarCAN_BroadcastOK();
                LED_State_t leds = { .charging = true, .hv_active = true };
                LEDSet(&leds);
            }
        }
        //something is wrong with Elcon CAN communication
        else if (elcon_result == CAN_ERR)
        {
            Elcon_SendChargeCommand(0.0f, 0.0f, true, portMAX_DELAY);
            CarCAN_BroadcastFault();
            faultBits_set(FAULT_ELCONCAN_HEARTBEAT_MISSED);
            LED_State_t leds = { .fault = true };
            LEDSet(&leds);
        }
        // no message yet — check if Elcon has gone silent too long
        else if ((xTaskGetTickCount() - xLastElcon_Tick) > pdMS_TO_TICKS(ELCON_STATUS_TIMEOUT_MS))
        {
            Elcon_SendChargeCommand(0.0f, 0.0f, true, portMAX_DELAY);
            CarCAN_BroadcastFault();
            faultBits_set(FAULT_ELCONCAN_HEARTBEAT_MISSED);
            LED_State_t leds = { .fault = true };
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
    faultBits_init();

    MX_GPIO_Init();
    MX_SPI3_Init();

    ElconCAN_Init();
    CarCAN_Init();

    Display_Init();

    vTaskDelete(NULL);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    ChargerTask_Handle = xTaskCreateStatic(
        Charger_Task,
        "Charger Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        ChargerTaskStack,
        &ChargerTask_Buffer);

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

    while (1) {}

    return 0;
}
