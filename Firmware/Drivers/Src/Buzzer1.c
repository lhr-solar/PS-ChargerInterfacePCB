#include "Buzzer1.h"
#include "common.h"
#include "pinDef.h"



//alarm states
static bool buzzer_active = false;
static uint32_t buzzer_end_ms = 0;

static uint32_t Buzzer_GetTimer(void){

    uint32_t pclk = HAL_RCC_GetPCLK1Freq();

    RCC_ClkInitTypeDef clk;
    uint32_t flashLatency;
    HAL_RCC_GetClockConfig(&clk, &flashLatency);

    //checks if APB is running the same as HCLK
    if (clk.APB1CLKDivider == RCC_HCLK_DIV1)
    {
        return pclk;
    }
    else
    {
        return pclk * 2U;
    }
}


void Buzzer_Init(void)
{
    // starts OFF
    Buzzer_Off();
}


//need to validate logic through testing on Nucleo Board

void Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms)
{
    if (freq_hz == 0 || duty_pct == 0 || duty_pct > 100) {
        Buzzer_Off();
        return;
    }

    uint32_t timer_clock = Buzzer_GetTimer(); // Get timer clock frequency
    uint32_t prescalar = BUZZER_TIMER.Instance->PSC;
    uint32_t tick_freq = timer_clock / (prescalar + 1); // Timer tick frequency 

    uint32_t arr = (tick_freq / freq_hz) - 1; // Auto-reload value
    if (arr = 0){
        arr = 1;
    }

    BUZZER_TIMER.Instance->ARR = arr;

    //duty cycle = CCR / (ARR+1)

    uint32_t ccr = ((arr + 1) * duty_pct) / 100;

    if (ccr > arr){
        ccr = arr;
    }

    //program CCR register for the channel
    __HAL_TIM_SET_COMPARE(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL, ccr);

    //start PWM
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
    Buzzer_Tone(1000, 60, 120); 

    Buzzer_Update();

    Buzzer_Tone(1000, 60, 120); 

    Buzzer_Update();

    Buzzer_Off();
}

// finished charging: 800 Hz 180 ms, gap 100 ms, 1.2 kHz 180 ms, gap 100 ms, 1.6 kHz 300 ms
void Buzzer_PlayFinish(void)
{
    Buzzer_Tone(800, 50, 90); 
    HAL_Delay(100);
    Buzzer_Off(); 

    Buzzer_Tone(1200, 60, 90); 
    HAL_Delay(100);
    Buzzer_Off();    

    Buzzer_Tone(1600, 70, 90); 
    HAL_Delay(100);
    Buzzer_Off();
}

// kaboom sound: fast variable between 2.4 kHz and 1.8 kHz — blocking
void Buzzer_Alarm(void) 
{
    s_alarm = true;

    if (!s_alarm) {
        return;
    }

    while (s_alarm) {
        Buzzer_Tone(2400, 90, 500); 
        HAL_Delay(50);
        Buzzer_Off();

        Buzzer_Tone(1800, 90, 500); 
        HAL_Delay(50);
        Buzzer_Off();
    }
}
