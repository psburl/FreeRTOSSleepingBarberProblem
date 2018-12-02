#include "FreeRTOS_AVR.h"
#include "basic_io_avr.h"

#define Number_Of_Chairs 2
#define Number_Of_Customers 5

QueueHandle_t baberQueueToCutHair;

void setup() {

    __malloc_heap_end = (char*)RAMEND;
    Serial.begin(9600);

    baberQueueToCutHair = xQueueCreate( Number_Of_Chairs, sizeof(TaskHandle_t));
    xTaskCreate(Barber, "barber", 200, NULL, 1,  NULL);

    for(int id = 1; id<= Number_Of_Customers; id++){
        xTaskCreate(Customer, "customer", 100, &id, 1,  NULL);
    }

    vTaskStartScheduler();

    for( ;; );
}

void loop() {}

void Barber(void* parameters){

    TickType_t delayTime = pdMS_TO_TICKS(100);
    portBASE_TYPE xStatus;
    TaskHandle_t customerTaskHandle;

    while(true){
       
        if(xQueueReceive(baberQueueToCutHair, &customerTaskHandle, portMAX_DELAY ) == pdPASS){
            vPrintString("The barber is cutting a hair\n");
            WaitRandTime(500);
            vPrintString("The barber has finished cutting hair.\n");
            xTaskNotifyGive(customerTaskHandle);
        }
        else
          vPrintString("The barber is sleeping\n");
    }
}

void Customer(void* parameters){

    TickType_t waitTime = pdMS_TO_TICKS(500);

    while(true){

        WaitRandTime(500);
        vPrintString("Customer arrived at barber shop.\n");
        TaskHandle_t customerTaskHandle = xTaskGetCurrentTaskHandle(); 

        if(xQueueSendToBack(baberQueueToCutHair, &customerTaskHandle, waitTime) == pdPASS){
            vPrintString("Customer entered on waiting room.\n");
            ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
            vPrintString("Customer leaving barber shop.\n");
        }
        else
            vPrintString("Customer leaving barber shop without cut.\n");
    }
}

void WaitRandTime(int maximumTime){
    vTaskDelay(rand()%maximumTime);
}