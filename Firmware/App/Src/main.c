#include "FreeRTOS.h"
#include "stm32xx_hal.h"
#include "common.h"
#include "pinDef.h"
#include "StatusLED.h"
#include "Buzzer1.h"
#include "Estop.h"



int main() {
    HAL_Init();
    Sys_Clock_Config();

    // Start the scheduler
    vTaskStartScheduler();

    error_handler();
    while (1) {
        // Scheduler should've started by now
        // Code should never enter this point
    }

    return 0;
}