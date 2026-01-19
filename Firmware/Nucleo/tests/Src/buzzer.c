#include "buzzer.h"
#include "pinDef.h"
#include <stdbool.h>
#include "stm32g4xx_hal.h" // adjust if your MCU family differs

static bool buzzer_active = false;
static uint32_t buzzer_end_ms = 0;

static bool s_alarm = false;

static uint32_t Buzzer_GetTimer(void)
{
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    RCC_ClkInitTypeDef clk;
    uint32_t latency;
    HAL_RCC_GetClockConfig(&clk, &latency);

    // If APB2 prescaler != 1, TIM1 clock = 2 * PCLK2, else = PCLK2
    return (clk.APB2CLKDivider == RCC_HCLK_DIV1) ? pclk2 : (2U * pclk2);
}


void Buzzer_Init(void)
{
    Buzzer_Off();
}


void Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms)
{
    if (freq_hz == 0 || duty_pct == 0 || duty_pct > 100) {
        Buzzer_Off();
        return;
    }

    uint32_t timer_clock = Buzzer_GetTimer();

    uint32_t prescaler = BUZZER_TIMER_HANDLE->Instance->PSC;
    uint32_t tick_freq = timer_clock / (prescaler + 1U);

    uint32_t arr = (tick_freq / (uint32_t)freq_hz);
    if (arr == 0U) {
        arr = 1U;
    }
    arr -= 1U;

    uint32_t ccr = ((arr + 1U) * (uint32_t)duty_pct) / 100U;
    if (ccr > arr) {
        ccr = arr;
    }

    (void)HAL_TIM_PWM_Stop(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);

    BUZZER_TIMER_HANDLE->Instance->ARR = (uint32_t)arr;

    __HAL_TIM_SET_COMPARE(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL, ccr);

    __HAL_TIM_SET_COUNTER(BUZZER_TIMER_HANDLE, 0U);

    HAL_TIM_GenerateEvent(BUZZER_TIMER_HANDLE, TIM_EVENTSOURCE_UPDATE);

    // Start PWM once
    HAL_TIM_PWM_Start(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);

    buzzer_active = true;

    if (duration_ms > 0) {
        buzzer_end_ms = HAL_GetTick() + duration_ms;
    } 
    else {
        buzzer_end_ms = 0;
    }

}

void Buzzer_Off(void)
{
    HAL_TIM_PWM_Stop(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);

    buzzer_active = false;
    buzzer_end_ms = 0;

}

void Buzzer_Update(void)
{
    if (!buzzer_active)
        return;

    if (buzzer_end_ms > 0)
    {
        uint32_t now = HAL_GetTick();

        // signed subtraction handles wrap-around
        if ((int32_t)(now - buzzer_end_ms) >= 0)
        {
            Buzzer_Off();
        }
    }
}


// starting charging sound: 1.0 kHz 120 ms, 90 ms gap, 1.0 kHz 120 ms
void Buzzer_PlayStart(void)
{
    // 1.0 kHz 120 ms, 90 ms gap, 1.0 kHz 120 ms (blocking)
    Buzzer_Tone(1000, 60, 0);
    HAL_Delay(120);
    Buzzer_Off();

    HAL_Delay(90);

    Buzzer_Tone(1000, 60, 0);
    HAL_Delay(120);
    Buzzer_Off();
}

// finished charging: 800 Hz 180 ms, gap 100 ms, 1.2 kHz 180 ms, gap 100 ms, 1.6 kHz 300 ms
void Buzzer_PlayFinish(void)
{
    // 800 Hz 180 ms, gap 100 ms, 1.2 kHz 180 ms, gap 100 ms, 1.6 kHz 300 ms (blocking)
    Buzzer_Tone(800, 50, 0);
    HAL_Delay(180);
    Buzzer_Off();
    HAL_Delay(100);

    Buzzer_Tone(1200, 60, 0);
    HAL_Delay(180);
    Buzzer_Off();
    HAL_Delay(100);

    Buzzer_Tone(1600, 70, 0);
    HAL_Delay(300);
    Buzzer_Off();
}

// kaboom sound: fast variable between 2.4 kHz and 1.8 kHz — blocking
void Buzzer_Alarm(void)
{
    // Blocking alarm: alternates tones until s_alarm is cleared.
    s_alarm = true;

    while (s_alarm)
    {
        Buzzer_Tone(2400, 90, 0);
        HAL_Delay(50);
        Buzzer_Off();

        Buzzer_Tone(1800, 90, 0);
        HAL_Delay(50);
        Buzzer_Off();
    }
}
