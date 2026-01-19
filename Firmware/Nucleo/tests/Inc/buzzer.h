#ifndef BUZZER_H
#define BUZZER_H
#include <stdint.h>
#include <stdbool.h>
#include "pinDef.h"
#include "tim.h"

#define BUZZER_CHANNEL      TIM_CHANNEL_1
#define BUZZER_TIMER_HANDLE (&htim2)

typedef enum {
    BUZZER_PATTERN_NONE = 0,
    BUZZER_PATTERN_KABOOM,
    BUZZER_PATTERN_FINISH,
    BUZZER_PATTERN_START,
} buzzer_pattern_t;


void Buzzer_Init(void);

void Buzzer_Tone(uint16_t freq_hz, uint8_t duty_pct, uint32_t duration_ms);  
void Buzzer_Off(void);
void Buzzer_Update(void);

void Buzzer_PlayStart(void);
void Buzzer_PlayFinish(void);
void Buzzer_Alarm(void);
void Buzzer_AlarmOff(void);


#endif