#ifndef BUZZER_H
#define BUZZER_H

#include "stm32xx_hal.h"
#include "FreeRTOS.h"
#include "pinDef.h"

typedef enum {
    BUZZER_PATTERN_NONE = 0,
    BUZZER_PATTERN_KABOOM,
    BUZZER_PATTERN_FINISH,
    BUZZER_PATTERN_START,
} buzzer_pattern_t;

/*
#define BUZZER_PORT GPIOB
#define BUZZER_PIN GPIO_PIN_12
*/


void Buzzer_Init(void);

void Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms);  
void Buzzer_Off(void);

void Buzzer_PlayStart(void);
void Buzzer_PlayFinish(void);
void Buzzer_Alarm(void);
void Buzzer_AlarmOff(void);


#endif