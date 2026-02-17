#include "DisplaySPI.h"
#include "spi.h"
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"
#include "main.h"
#include "StatusLED.h"
#include "gpio.h"
#include "stm32g4xx_hal.h"


TaskHandle_t DisplayTestTask_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;

StaticTask_t DisplayTestTask_Buffer;
StackType_t DisplayTestTaskStack[512];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];


// test pattern: checkerboard pattern (verifies SPI + all control pins)
// renders text on all 4 lines 
// change contrast from min to max
// turning display on and off
void DisplayTestTask(void* argument)
{
    Display_Init();

    Display_TestPattern();
    vTaskDelay(pdMS_TO_TICKS(2000));

    Display_Clear();
    vTaskDelay(pdMS_TO_TICKS(500));

    Display_DrawString(0, 0, "lil shah");
    Display_Update();
    
    
    

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void HeartBeatTask(void* argument)
{
    HeartBeat();

}


int main(void)
{
    HAL_Init();
    SystemClock_Config();


    MX_GPIO_Init();
    MX_SPI3_Init();

    


    
    DisplayTestTask_Handle = xTaskCreateStatic(
        DisplayTestTask,
        "Display Test",
        512,
        NULL,
        tskIDLE_PRIORITY + 2,
        DisplayTestTaskStack,
        &DisplayTestTask_Buffer);
    

    HeartBeatTask_Handle = xTaskCreateStatic(
        HeartBeatTask,
        "Heartbeat LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        HeartBeatTaskStack,
        &HeartBeatTask_Buffer);

    vTaskStartScheduler();

    while (1) {


        vTaskDelay(pdMS_TO_TICKS(1000));

    }

    return 0;
}
