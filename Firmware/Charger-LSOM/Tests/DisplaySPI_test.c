#include "DisplaySPI.h"
#include "spi.h"
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "common.h"


TaskHandle_t DisplayTestTask_Handle = NULL;

StaticTask_t DisplayTestTask_Buffer;
StackType_t DisplayTestTaskStack[configMINIMAL_STACK_SIZE * 2];


// test pattern: checkerboard pattern (verifies SPI + all control pins)
// renders text on all 4 lines 
// change contrast from min to max
// turning display on and off
void DisplayTestTask(void* argument)
{
    Display_Init();

    //checkerboard pattern
    Display_TestPattern();
    vTaskDelay(pdMS_TO_TICKS(2000));

    //text all 4 lines
    Display_Clear();
    Display_DrawString(0, 0, "ABCabc0123:.,-");
    Display_Update();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    Display_Clear();
    Display_DrawString(0, 0, "bullshit ass display");
    Display_Update();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_SPI2_Init();

    DisplayTestTask_Handle = xTaskCreateStatic(
        DisplayTestTask,
        "Display Test",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        tskIDLE_PRIORITY + 1,
        DisplayTestTaskStack,
        &DisplayTestTask_Buffer);

    vTaskStartScheduler();

    while (1) {
    }

    return 0;
}
