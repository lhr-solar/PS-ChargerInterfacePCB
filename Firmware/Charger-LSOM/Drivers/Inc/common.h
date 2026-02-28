#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "stm32xx_hal.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "StatusLED.h"

#define FAULT_MESSAGE_DELAY pdMS_TO_TICKS(200)
#define ALL_FAULT_BITS ((1UL << NUM_FAULTS) - 1UL); // creates a bitmask with all fault bits set to 1 based on the total number of faults defined in the enum
#define MAX_FAULT_BITS 24U // use 32-bit unsigned integer for fault state to allow for more than 8 faults


typedef enum
{
  FAULT_ESTOP, // estop pressed

  FAULT_ELCON_UV,         // elcon undervoltage
  FAULT_ELCON_OV,         // elcon overvoltage
  FAULT_ELCON_OC,         // elcon overcurrent
  FAULT_ELCON_HARDWARE,   // elcon hardware failure (over temp, fan failure, etc)
  FAULT_ELCON_OVERTEMP,   // elcon over temperature
  FAULT_ELCON_INPUT_VOLT, // elcon input voltage wrong

  FAULT_BPS_OV,        // bps overvoltage
  FAULT_BPS_OC,        // bps overcurrent
  FAULT_BPS_UV,       // bps undervoltage
  FAULT_BPS_OVERTEMP, // bps over temperature

  FAULT_DISPLAY, // display failure (SPI failures)
  FAULT_BUZZER,  // buzzer driver malfunction (PWM/timer failures)

  FAULT_CARCAN_HEARTBEAT_MISSED,  // missed heartbeat from carCAN
  FAULT_ELCONCAN_HEARTBEAT_MISSED, // missed heartbeat from elconCAN

  NUM_FAULTS //to check total # of faults 

} fault_state_t;  



#define FAULT_BIT(fault) (1UL << (fault)) // macro to convert fault enum value to corresponding bit position in the fault bitmap
_Static_assert(NUM_FAULTS <= MAX_FAULT_BITS, "too many fault bits");

uint8_t faultBits_init(void);


/**
 * @brief   Utilize the fault enum to set fault to a certain state. This function can be used to set multiple faults.
 * @param   fault The fault state(s) to set
 * @return  Sets the specified fault(s) in the system fault state
 */

void faultBits_set(fault_state_t inputBit);

bool faultBit_wait(fault_state_t inputBit, TickType_t xTicksToWait);

bool faultBits_isSet(fault_state_t fault);

void faultBits_clear(fault_state_t fault);




void SystemClock_Config(void);

void Error_Handler(void);
