#include "Buzzer1.h"
#include "common.h"
#include "tim.h"
#include <stdlib.h>
#include "pinDef.h"

//states of variables
bool buzzer_active = false;
bool s_alarm = false;
GPIO_PinState estop_state = GPIO_PIN_RESET;

static void Buzzer_SetTone(uint16_t freq_hz, uint8_t duty_pct) {
    if (freq_hz == 0 || duty_pct == 0 || duty_pct > 100) {
        Buzzer_Off();
        return;
    }

    uint32_t psc = BUZZER_TIMER_HANDLE->Instance->PSC;        // prescaler register
    uint32_t tim_clk = BUZZER_TIMER_CLK_HZ/ (psc+1UL);      // timer counting rate

    uint32_t arr = (tim_clk / freq_hz) - 1UL;   

    uint32_t ccr = ((arr + 1UL) * duty_pct)/ 100UL;

    if (ccr > arr){
        ccr = arr;
    }
    
    __HAL_TIM_SET_AUTORELOAD(BUZZER_TIMER_HANDLE, arr);
    __HAL_TIM_SET_COMPARE(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL, ccr);
    HAL_TIM_GenerateEvent(BUZZER_TIMER_HANDLE, TIM_EVENTSOURCE_UPDATE);
    
    
}

void Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms)
{
    Buzzer_SetTone(freq_hz, duty_pct);
    Buzzer_Off();
}


void Buzzer_Init(void)
{
    HAL_TIM_PWM_Start(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);
    Buzzer_Off();
    buzzer_active = false;
}

void Buzzer_Off(void)
{
    __HAL_TIM_SET_COMPARE(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL, 0);
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
    buzzer_active = false;

}


// starting charging sound: 
void ChargeStart(void)
{
    Buzzer_Tone(200, 25, 2000); 

    Buzzer_Tone(1000, 75, 2000); 

}

// finished charging:
void ChargeStop(void)
{
    Buzzer_Tone(800, 50, 5000); 

    Buzzer_Tone(1200, 60, 5000); 

    Buzzer_Tone(1600, 70, 5000); 

}

// kaboom sound: fast variable between 2.4 kHz and 1.8 kHz
void Alarm(void) 
{
    
    s_alarm = true;

    static bool hi = true;

    if (hi) {
        Buzzer_Tone(2400, 90, 500); 
        hi = false;
    } else {
        Buzzer_Tone(1800, 90, 500); 
        hi = true;
    }

}

/*

while (s_alarm) {


        Buzzer_Tone(2400, 90, 500); 

        Buzzer_Tone(1800, 90, 500); 
    

        estop_state = HAL_GPIO_ReadPin(Estop_GPIO_Port, Estop_Pin);

        //if Estop is pressed, turn off alarm
        if (estop_state == GPIO_PIN_SET) {
            s_alarm = false;
            Buzzer_Off();
        }
    }



*/


//final PWM killer
void PWM_Stop(void)
{
    HAL_TIM_PWM_Stop(BUZZER_TIMER_HANDLE, BUZZER_CHANNEL);

    buzzer_active = false;
}

