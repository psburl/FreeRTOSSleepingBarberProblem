/* Federal University of Fronteira Sul - 2018
  Wesley Burlani

  Implementation of sleeping barber problem on FreeRTOS 
  using only queue structure and semaphores to control parallel flow

  Details: this implementation uses 1 queue of <MAX_WAIT_SEATS> elemens
  to control until 5 customers, where each queue element represents 1 forks

The analogy is based upon a hypothetical barber shop with one barber. 
The barber has one barber's chair in a cutting room and a waiting room
containing a number of chairs in it. When the barber finishes cutting
a customer's hair, he dismisses the customer and goes to the waiting
room to see if there are others waiting. If there are, he brings one
of them back to the chair and cuts their hair. If there are none,
he returns to the chair and sleeps in it.

Each customer, when they arrive, looks to see what the barber is doing.
If the barber is sleeping, the customer wakes him up and sits in the
cutting room chair. If the barber is cutting hair, the customer stays
in the waiting room. If there is a free chair in the waiting room, the
customer sits in it and waits their turn. If there is no free chair,
the customer leaves. 
*/

#include "FreeRTOS_AVR.h"
#include "basic_io_avr.h"

#define Number_Of_Chairs 2
#define Number_Of_Customers 5

// limits the # of customers allowed to enter on waiting room
SemaphoreHandle_t chairsSemaphore;
// ensures exclusive access tothe barber chair
SemaphoreHandle_t barberChairSemaphore;
// allows barber to sleep until a customer arrives
SemaphoreHandle_t barberCanSleepSemaphore;
// make the customer to wait until the barber is done cutting his/her hair. 
SemaphoreHandle_t customerCanGoAwaySemaphore;



void setup() {

    __malloc_heap_end = (char*)RAMEND;
    Serial.begin(9600);

    BaseType_t xStatus;
    vSemaphoreCreateBinary(barberCanSleepSemaphore);
    vSemaphoreCreateBinary(customerCanGoAwaySemaphore);
    vSemaphoreCreateBinary(barberChairSemaphore);
    chairsSemaphore = xSemaphoreCreateCounting(Number_Of_Chairs, Number_Of_Chairs);
    xSemaphoreTake(customerCanGoAwaySemaphore, portMAX_DELAY);
    xSemaphoreTake(barberCanSleepSemaphore, portMAX_DELAY);

    xTaskCreate(Barber, "barber", 200, NULL, 1,  NULL);

    for(int id = 1; id<= Number_Of_Customers; id++){
        xTaskCreate(Customer, "customer", 100, &id, 1,  NULL);
    }

    vTaskStartScheduler();

    for( ;; );
}

void loop() {}

void Barber(void* parameters){

    while(true){
        
        vPrintString("The barber is sleeping\n");
        // barber will sleep until a customer gets this mutex
        xSemaphoreTake(barberCanSleepSemaphore,portMAX_DELAY );

        vPrintString("The barber is cutting a hair\n");
        WaitRandTime(500);
        vPrintString("The barber has finished cutting hair.\n");
        xSemaphoreGive(customerCanGoAwaySemaphore);
        
    }
}


void Customer(void* parameters){

    TickType_t waitTime = pdMS_TO_TICKS(500);

    while(true){

        WaitRandTime(500);
        vPrintString("Customer arrived at barber shop.\n");
        if(xSemaphoreTake(chairsSemaphore, waitTime) == pdTRUE){
          
            vPrintString("Customer entered on waiting room.\n");
            // customer waits until barber is cutting other hair
            xSemaphoreTake(barberChairSemaphore, portMAX_DELAY);
            // when customer see baber chair free he leaves your chair
            xSemaphoreGive(chairsSemaphore);
            xSemaphoreGive(barberCanSleepSemaphore);
            xSemaphoreTake(customerCanGoAwaySemaphore, portMAX_DELAY);
            xSemaphoreGive(barberChairSemaphore);
            vPrintString("Customer leaving barber shop.\n");
        }
        else
            vPrintString("Customer leaving barber shop without cut.\n");
    }
}

void WaitRandTime(int maximumTime){
    vTaskDelay(rand()%maximumTime);
}