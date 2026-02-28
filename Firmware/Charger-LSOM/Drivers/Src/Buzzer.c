
/* Copyright (c) 2018-2026 UT Longhorn Racing Solar */
/** Buzzer.c
 * Wrapper that holds all buzzer/PWM related functionality.
 */

#include "Buzzer.h"
#include "common.h"
#include "tim.h"
#include <stdlib.h>
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

SemaphoreHandle_t BuzzerMutex = NULL;

StaticSemaphore_t BuzzerMutexBuffer;

// states of variables
bool alarm_status = false;
GPIO_PinState estop_state = GPIO_PIN_RESET;

static Buzzer_status_t Buzzer_SetTone(uint16_t freq_hz, uint8_t duty_pct)
{
    if (freq_hz == 0 || duty_pct == 0 || duty_pct > 100)
    {
        printf("Invalid buzzer parameters: freq=%u Hz, duty=%u%%\r\n", freq_hz, duty_pct);

        Buzzer_Off();
        return BUZZER_ERROR_INVALID_PARAM;
    }

    uint32_t psc = BUZZER_TIMER_HANDLE->Instance->PSC;    // prescaler register
    uint32_t tim_clk = BUZZER_TIMER_CLK_HZ / (psc + 1UL); // timer counting rate

    // calculates arr and ccr values for desired frequency and duty cycle
    // arr controls the period of the PWM signal
    // ccr controls the duty cycle (the time spent in the high state within one period)

    uint32_t arr = (tim_clk / freq_hz) - 1UL;

    uint32_t ccr = ((arr + 1UL) * duty_pct) / 100UL;

    // safety check to ensure ccr does not exceed arr, which would cause an invalid PWM signal
    if (ccr > arr)
    {
        ccr = arr;
    }

    // update timer registers with new values and generate an update event to apply changes
    __HAL_TIM_SET_AUTORELOAD(BUZZER_TIMER_HANDLE, arr);
    __HAL_TIM_SET_COMPARE(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL, ccr);
    HAL_TIM_GenerateEvent(BUZZER_TIMER_HANDLE, TIM_EVENTSOURCE_UPDATE);

    return BUZZER_OK;
}

Buzzer_status_t Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms)
{
    Buzzer_SetTone(freq_hz, duty_pct);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    Buzzer_Off();
    return BUZZER_OK;
}

void Buzzer_Init(void)
{

    BuzzerMutex = xSemaphoreCreateMutexStatic(&BuzzerMutexBuffer);

    HAL_TIM_PWM_Start(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);
    Buzzer_Off();
}

void Buzzer_Off(void)
{
    __HAL_TIM_SET_COMPARE(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL, 0);
    HAL_GPIO_WritePin(BUZZPWM_PORT, BUZZPWM_PIN, GPIO_PIN_RESET);
}

// starting charging sound:
void Buzzer_ChargeStart(void)
{

    if (xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE)
    {
        Buzzer_Tone(BUZZER_CHARGE_START_FREQ_HZ, BUZZER_CHARGE_START_DUTY_PCT, BUZZER_CHARGE_START_DURATION_MS);

        Buzzer_Tone(BUZZER_CHARGE_START_FREQ_HZ_2, BUZZER_CHARGE_START_DUTY_PCT_2, BUZZER_CHARGE_START_DURATION_MS_2);



        xSemaphoreGive(BuzzerMutex);
    }
}

// finished charging:
void Buzzer_ChargeStop(void)
{

    if (xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE)
    {
        Buzzer_Tone(BUZZER_CHARGE_STOP_FREQ_HZ_1, BUZZER_CHARGE_STOP_DUTY_PCT_1, BUZZER_CHARGE_STOP_DURATION_MS_1);

        Buzzer_Tone(BUZZER_CHARGE_STOP_FREQ_HZ_2, BUZZER_CHARGE_STOP_DUTY_PCT_2, BUZZER_CHARGE_STOP_DURATION_MS_2);

        Buzzer_Tone(BUZZER_CHARGE_STOP_FREQ_HZ_3, BUZZER_CHARGE_STOP_DUTY_PCT_3, BUZZER_CHARGE_STOP_DURATION_MS_3);
        xSemaphoreGive(BuzzerMutex);
    }
}

// kaboom sound: fast variable between 2.4 kHz and 1.8 kHz
void Buzzer_ChargeAlarm(void)
{

    estop_state = HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN);
    //TODO: Update to have more fault states that will cause an alarm and not just Estop, later
    alarm_status = (estop_state == GPIO_PIN_RESET) ? true : false; // active low

    while (alarm_status)
    {
        if (xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE)
        {
            Buzzer_Tone(BUZZER_CHARGE_ALARM_FREQ_HZ_1, BUZZER_CHARGE_ALARM_DUTY_PCT_1, BUZZER_CHARGE_ALARM_DURATION_MS_1);

            Buzzer_Tone(BUZZER_CHARGE_ALARM_FREQ_HZ_2, BUZZER_CHARGE_ALARM_DUTY_PCT_2, BUZZER_CHARGE_ALARM_DURATION_MS_2);

            xSemaphoreGive(BuzzerMutex);
        }

        estop_state = HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN);
        if (estop_state == GPIO_PIN_SET)
        {
            alarm_status = false;
            Buzzer_Off();
        }
    }
}

// Stops all PWM output immediately
void PWM_Stop(void)
{
    HAL_TIM_PWM_Stop(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);
}
