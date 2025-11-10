#include "Buzzer1.h"
#include "common.h"
#include "stm32xx_hal.h"
#include "pinDef.h"


//alarm states
static bool s_alarm = false;

void Buzzer_Init(void)
{
    // Configure the GPIO pin as output push-pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = BUZZER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);

    // Start off
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

//need to validate through testing on Nucleo Board

void Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms)
{
    if (freq_hz == 0 || duty_pct == 0){
        return;
    }

    float period_ms = 1000.0f / freq_hz;              // Period in miliseconds
    float on_time = period_ms * (duty_pct / 100.0f);     // ON duration (us)
    float off_time = period_ms - on_time;                // OFF duration (us)
    uint32_t cycles = (duration_ms * 1000) / period_ms;  // How many cycles to run

    for (uint32_t i = 0; i < cycles; i++) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
        HAL_Delay(uint32_t(on_time));   
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        HAL_Delay(uint32_t(off_time));
    }
}

void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}


// starting charging sound: 1.0 kHz 120 ms, 90 ms gap, 1.0 kHz 120 ms
void Buzzer_PlayStart(void)
{
    Buzzer_Tone(1000, 60, 120); 

    HAL_Delay(100);
    Buzzer_Off();

    Buzzer_Tone(1000, 60, 120); 

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

void Buzzer_AlarmOff(void)
{
    s_alarm = false;
    Buzzer_Off();
}
