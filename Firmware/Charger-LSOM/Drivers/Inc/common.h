#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"
#include "tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "StatusLED.h"

#define FAULT_MESSAGE_DELAY pdMS_TO_TICKS(200)

typedef enum
{
  FAULT_NONE = 0, // no active faults

  FAULT_ESTOP = 1 << 1, // estop pressed

  FAULT_ELCON_UV = 1 << 2, // elcon undervoltage
  FAULT_ELCON_OV = 1 << 3, // elcon overvoltage
  FAULT_ELCON_OC = 1 << 4, // elcon overcurrent

  FAULT_BPS_OV = 1 << 8, // bps overvoltage
  FAULT_BPS_OC = 1 << 9, // bps overcurrent

  FAULT_DISPLAY = 1 << 10, // display failure (SPI failures)
  FAULT_BUZZER = 1 << 11,  // buzzer driver malfunction (PWM/timer failures)

  FAULT_HEARTBEAT_MISSED = 1 << 12 // missed heartbeat from car (no CAN messages received for a certain period)
} fault_state_t;

/**
 * @brief   Utilize the fault enum to set fault to a certain state. This function can be used to set multiple faults.
 * @param   fault The fault state(s) to set
 * @return  Sets the specified fault(s) in the system fault state
 */

void Fault_Set(fault_state_t fault);

/**
 * @brief   Utilize the fault enum to clear fault to a certain state. This function can be used to clear multiple faults.
 * @param   fault The fault state(s) to clear
 * @return  Clears the specified fault(s) in the system fault state
 */
void Fault_Clear(fault_state_t fault);

/**
 * @brief   Checks if a specific fault is currently set
 * @param   fault The fault state to check
 * @return  true if the specified fault is currently set, false otherwise
 */
bool Fault_IsSet(fault_state_t fault);

/**
 * @brief   Retrieves the current fault bitmap representing all active faults in the system
 * @param   none
 * @return  A 32-bit unsigned integer where each bit represents a different fault state. A bit value of 1 indicates the corresponding fault is active, while a bit value of 0 indicates it is not.
 */
uint32_t Fault_GetAll(void);

void SystemClock_Config(void);

void Error_Handler(void);