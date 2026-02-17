#include "Buzzer.h"
#include "tim.h"
#include <stdlib.h>
#include "pinDef.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "common.h"
#include "StatusLED.h"


TaskHandle_t BuzzerTask_Charging_Handle = NULL;
TaskHandle_t BuzzerTask_Alarm_Handle = NULL;
TaskHandle_t HeartBeatTask_Handle = NULL;


StaticTask_t BuzzerTask_Charging_Buffer;
StackType_t BuzzerTask_ChargingStack[configMINIMAL_STACK_SIZE];
StaticTask_t BuzzerTask_Alarm_Buffer;
StackType_t BuzzerTask_AlarmStack[configMINIMAL_STACK_SIZE];

StaticTask_t HeartBeatTask_Buffer;
StackType_t HeartBeatTaskStack[configMINIMAL_STACK_SIZE];

SemaphoreHandle_t BuzzerMutex;
StaticSemaphore_t BuzzerMutexBuffer;



void BuzzerTask_Charging(void* argument){
    
    while(1){
        if(xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE) {
            ChargeStart();
            vTaskDelay(pdMS_TO_TICKS(2000));
            ChargeStop();
            xSemaphoreGive(BuzzerMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void BuzzerTask_Alarm(void* argument){

    while(1){
        if(xSemaphoreTake(BuzzerMutex, portMAX_DELAY) == pdTRUE) {
            ChargeAlarm();
            
            xSemaphoreGive(BuzzerMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void HeartBeatTask(void* argument)
{
    HeartBeat();
}


int main (void) {

    HAL_Init();
    SystemClock_Config();
    MX_TIM5_Init();
    
    Buzzer_Init();


    BuzzerMutex = xSemaphoreCreateMutexStatic(&BuzzerMutexBuffer);

   


    BuzzerTask_Alarm_Handle=xTaskCreateStatic(
        BuzzerTask_Alarm,
        "Buzzer Task Alarm",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        BuzzerTask_AlarmStack,
        &BuzzerTask_Alarm_Buffer);

    BuzzerTask_Charging_Handle=xTaskCreateStatic(
        BuzzerTask_Charging,
        "Buzzer Task Charging",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 3,
        BuzzerTask_ChargingStack,
        &BuzzerTask_Charging_Buffer);


    HeartBeatTask_Handle = xTaskCreateStatic(
        HeartBeatTask,
        "Heartbeat LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        HeartBeatTaskStack,
        &HeartBeatTask_Buffer);

    vTaskStartScheduler();


    while (1){


    }

    return 0; 

}
