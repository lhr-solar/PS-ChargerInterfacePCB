#pragma once

#include "stm32xx_hal.h"
#include "FreeRTOS.h"
#include "pinDef.h"
#include <stdlib.h>
#include "tim.h"

#define BUZZER_TIMER htim5
#define BUZZER_CHANNEL TIM_CHANNEL_1
#define BUZZER_TIMER_HANDLE (&BUZZER_TIMER)
#define BUZZER_TIMER_CLK_HZ 80000000UL

typedef enum {

    BUZZER_OK = 0,
    BUZZER_ERROR_INVALID_PARAM,
    BUZZER_ERROR_HW_FAILURE

}Buzzer_status_t; 




/**
 * @brief   Starts the PWM signal for the buzzer and turns it off by default
 */

void Buzzer_Init(void);

/**
 * @brief   Sets the Buzzer Tone, Freq, and Duration for a given buzzer utilizing PWM
 * @param   freq_hz Frequency of the buzzer tone in Hz
 * @param   duty_pct Duty cycle of the PWM signal in percentage (0 to 100)
 * @param   duration_ms Duration for which the tone should play in milliseconds
 * @return  outputs a tone at the specified frequency and duty cycle for the specified duration, then turns off the buzzer
 */

Buzzer_status_t Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms);

/**
 * @brief   Resets the buzzer by stopping the PWM signal and setting the output pin low
 */
void Buzzer_Off(void);


/**
 * @brief   Stops all PWM output immediately
 */
void PWM_Stop(void);

/**
 * @brief   Starts the charging sound sequence
 */
void ChargeStart(void);

/**
 * @brief   Starts the charging stop sound sequence
 */
void ChargeStop(void);

/**
 * @brief   Starts the charging alarm sound sequence
 */
void ChargeAlarm(void);
