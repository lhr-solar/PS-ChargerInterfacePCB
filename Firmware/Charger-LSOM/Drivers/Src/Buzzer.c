
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
        printf("Invalid buzzer parameters: freq=%u Hz, duty=%u%%\n", freq_hz, duty_pct);

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
void ChargeStart(void)
{

    if (xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE)
    {
        Buzzer_Tone(200, 25, 2000);

        Buzzer_Tone(1000, 75, 2000);

        vTaskDelay(pdMS_TO_TICKS(2000));

        xSemaphoreGive(BuzzerMutex);
    }
}

// finished charging:
void ChargeStop(void)
{

    if (xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE)
    {
        Buzzer_Tone(800, 50, 5000);

        Buzzer_Tone(1200, 60, 5000);

        Buzzer_Tone(1600, 70, 5000);

        vTaskDelay(pdMS_TO_TICKS(2000));

        xSemaphoreGive(BuzzerMutex);
    }
}

// kaboom sound: fast variable between 2.4 kHz and 1.8 kHz
void ChargeAlarm(void)
{

    estop_state = HAL_GPIO_ReadPin(ESTOP_PORT, ESTOP_PIN);
    alarm_status = (estop_state == GPIO_PIN_RESET) ? true : false; // active low

    while (alarm_status)
    {
        if (xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE)
        {
            Buzzer_Tone(2400, 90, 500);

            Buzzer_Tone(1800, 90, 500);

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
