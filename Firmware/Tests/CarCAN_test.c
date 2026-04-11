/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** CarCAN_test.c
 * Description: Integration task for CarCAN + ElconCAN charging control.
 *              Charges via Elcon when BPS grants permission (BPS_Charge_OK=1).
 *              Stops immediately and faults on BPS denial or any Elcon fault flag.
 *              Broadcasts charger telemetry and faults to CarCAN on ID 0xE
 *              (ChargerInterface_Status) instead of spoofing BPS_Status_ID.
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
#include "ElconCAN.h"

TaskHandle_t ChargerTask_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;
TaskHandle_t InitTask_Handle = NULL;

StaticTask_t ChargerTask_Buffer;
StackType_t ChargerTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

StaticTask_t InitTask_Buffer;
StackType_t InitTaskStack[configMINIMAL_STACK_SIZE];

// Pack and send a charge command to the Elcon (CAN ID 0x1806E5F4)
// voltage_dv in 0.1V units, current_da in 0.1A units
static can_status_t Elcon_SendChargeCommand(uint16_t voltage_dv, uint16_t current_da, bool stop, TickType_t delay_ticks)
{
    uint8_t tx_data[8] = {0};
    tx_data[0] = (uint8_t)(voltage_dv >> 8);
    tx_data[1] = (uint8_t)(voltage_dv & 0xFF);
    tx_data[2] = (uint8_t)(current_da >> 8);
    tx_data[3] = (uint8_t)(current_da & 0xFF);
    tx_data[4] = (uint8_t)stop;

    return ElconCAN_Send(ELCONCAN_MSG_1_ID, tx_data, FDCAN_DLC_BYTES_8, delay_ticks);
}

void Charger_Task(void *argument)
{

    ElconStatus_t elcon_status = {0};
    CarCAN_BPS_Aggregate_t bps_agg = {0};
    uint8_t rx_data[8];
    char buf[32];
    can_status_t carcan_tx;
    bool bps_charge_ok = false;
    bool elcon_was_ok = false;
    TickType_t xLastBPS_Tick = xTaskGetTickCount();
    TickType_t xLastElcon_Tick = xTaskGetTickCount();

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(CHARGER_TASK_PERIOD_MS);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // Non-blocking check for BPS_Status — the safety gate for charging.
        // Timeout is tracked via xLastBPS_Tick below; blocking here would exceed the 250ms period.
        if (CarCAN_Receive_BPS_Status(rx_data, 0) == CAN_OK)
        {
            bps_charge_ok = (rx_data[BPS_STATUS_CHARGE_OK_BYTE] & BPS_STATUS_CHARGE_OK_MASK) == BPS_STATUS_BPS_CHARGE_OK_OK;
            xLastBPS_Tick = xTaskGetTickCount();
        }

        // Drain any voltage tap frames that arrived since last cycle.
        while (CarCAN_Receive_BPS_Voltage(rx_data) == CAN_OK)
        {
            CarCAN_Unpack_BPS_Aggregate(rx_data, &bps_agg);
            // TODO: min/max cell voltage checks
        }

        // BPS timeout check:
        // xTaskGetTickCount() returns the current FreeRTOS tick counter.
        // xLastBPS_Tick holds the tick count at the last received BPS_Status frame.
        // Subtracting them gives the time passed since last message.
        // If no BPS frame has arrived in 300ms (3 missed frames at 10Hz), bps_timeout is set.
        bool bps_timeout = (xTaskGetTickCount() - xLastBPS_Tick) > pdMS_TO_TICKS(BPS_STATUS_TIMEOUT_MS);

        // Charging is only allowed when BPS is alive AND explicitly grants permission.
        // Both conditions must be true; either failure disallows charging this cycle.
        bool bps_allow_charge = !bps_timeout && bps_charge_ok;

        // Set the specific fault before the shared stop-charging block below
        if (bps_timeout)
        {
            faultBits_set(FAULT_CARCAN_HEARTBEAT_MISSED);
        }
        else if (!bps_charge_ok)
        {
            faultBits_set(FAULT_BPS_CHARGE_NOT_OK);
        }

        if (!bps_allow_charge)
        {
            // Stop charging, drain any pending Elcon RX, broadcast fault status
            elcon_was_ok = false;
            Elcon_SendChargeCommand(0U, 0U, true, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
            {
                ElconStatus_t tmp;
                uint8_t tmp_data[8];
                while (ElconCAN_Receive(&tmp, ELCONCAN_RX_ID, tmp_data, 0) == CAN_OK)
                {
                }
            }
            carcan_tx = CarCAN_Send_ChargerInterface_Status(0U, 0U, ELCON_COMM_OK, ELCON_FAULT, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
            LED_State_t fault_leds = {.fault = true, .hv_active = (carcan_tx == CAN_OK)};
            LEDSet(&fault_leds);
        }
        else
        {
            // charging allowed, only send charge command if Elcon is healthy
            if (elcon_was_ok)
            {
                Elcon_SendChargeCommand(ELCON_TARGET_VOLTAGE_DV, ELCON_TARGET_CURRENT_DA, false, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
            }
            else
            {
                Elcon_SendChargeCommand(0U, 0U, true, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
            }

            // read elcon incoming data
            can_status_t elcon_result = ElconCAN_Receive(&elcon_status, ELCONCAN_RX_ID, rx_data, 0);
            snprintf(buf, sizeof(buf), "V:%u.%uV  I:%u.%uA",
                     elcon_status.output_voltage_dv / 10, elcon_status.output_voltage_dv % 10,
                     elcon_status.output_current_da / 10, elcon_status.output_current_da % 10);
            Display_DrawString(0, 8, buf);

            snprintf(buf, sizeof(buf), "HW:%d OT:%d IV:%d",
                     elcon_status.flag_hw_failure, elcon_status.flag_over_temp, elcon_status.flag_input_voltage_wrong);
            Display_DrawString(0, 16, buf);

            if (elcon_result == CAN_OK)
            {
                xLastElcon_Tick = xTaskGetTickCount();

                bool elcon_fault = elcon_status.flag_hw_failure || elcon_status.flag_over_temp || elcon_status.flag_input_voltage_wrong || elcon_status.flag_starting_state || elcon_status.flag_comm_timeout;

                // if any faults occur from Elcon side
                if (elcon_fault)
                {
                    elcon_was_ok = false;
                    carcan_tx = CarCAN_Send_ChargerInterface_Status(elcon_status.output_voltage_dv, elcon_status.output_current_da, ELCON_COMM_OK, ELCON_FAULT, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
                    faultBits_set(FAULT_ELCON_HARDWARE);
                    LED_State_t leds = {.fault = true, .hv_active = (carcan_tx == CAN_OK)};
                    LEDSet(&leds);
                }
                else
                {
                    elcon_was_ok = true;
                    carcan_tx = CarCAN_Send_ChargerInterface_Status(elcon_status.output_voltage_dv, elcon_status.output_current_da, ELCON_COMM_OK, ELCON_NO_FAULT, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
                    LED_State_t leds = {.charging = true, .hv_active = (carcan_tx == CAN_OK)};
                    LEDSet(&leds);
                }
            }
            // something is wrong with Elcon CAN communication
            else if (elcon_result == CAN_ERR)
            {
                elcon_was_ok = false;
                carcan_tx = CarCAN_Send_ChargerInterface_Status(0U, 0U, ELCON_COMM_FAULT, ELCON_FAULT, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
                faultBits_set(FAULT_ELCONCAN_HEARTBEAT_MISSED);
                LED_State_t leds = {.fault = true, .hv_active = (carcan_tx == CAN_OK)};
                LEDSet(&leds);
            }
            // no message yet — check if Elcon has gone silent too long
            else if ((xTaskGetTickCount() - xLastElcon_Tick) > pdMS_TO_TICKS(ELCON_STATUS_TIMEOUT_MS))
            {
                elcon_was_ok = false;
                carcan_tx = CarCAN_Send_ChargerInterface_Status(0U, 0U, ELCON_COMM_FAULT, ELCON_FAULT, pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
                faultBits_set(FAULT_ELCONCAN_HEARTBEAT_MISSED);
                LED_State_t leds = {.fault = true, .hv_active = (carcan_tx == CAN_OK)};
                LEDSet(&leds);
            }
            else
            {
                // CAN_EMPTY and Elcon not yet timed out — send last known status every cycle
                // so 0xE always goes out at 250ms regardless of Elcon's ~1Hz broadcast rate.
                carcan_tx = CarCAN_Send_ChargerInterface_Status(
                    elcon_status.output_voltage_dv, elcon_status.output_current_da,
                    elcon_was_ok ? ELCON_COMM_OK : ELCON_COMM_FAULT,
                    elcon_was_ok ? ELCON_NO_FAULT : ELCON_FAULT,
                    pdMS_TO_TICKS(CAN_TX_TIMEOUT_MS));
                LED_State_t leds = {.charging = elcon_was_ok, .hv_active = (carcan_tx == CAN_OK)};
                LEDSet(&leds);
            }
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

    while (1)
    {
    }

    return 0;
}
