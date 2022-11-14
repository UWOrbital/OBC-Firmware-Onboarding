#include "amb_light_service.h"
#include "serial_io.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <sys_common.h>

#include <adc.h>
#include <sci.h>
#include <stdio.h>
#include <string.h>


/* USER CODE BEGIN */
// Include any additional headers and global variables here
static TaskHandle_t lightServiceHandle = NULL;
static QueueHandle_t lightServiceQueue = NULL;

/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.
    BaseType_t xReturned = pdFAIL;
    if (lightServiceHandle == NULL) {
        // Create controller task
        xReturned = xTaskCreate(lightServiceTask,             /* Function that implements the task. */
                                LIGHT_SERVICE_NAME,            /* Text name for the task. */
                                LIGHT_SERVICE_STACK_SIZE,      /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                LIGHT_SERVICE_PRIORITY,        /* Priority at which the task is created. */
                                &lightServiceHandle);     /* Used to pass out the created task's handle. */
    }

    // create queue
    if(lightServiceQueue == NULL){
        lightServiceQueue = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH, LIGHT_SERVICE_ITEM_SIZE);
    }
    
    // Check if create queue failed
    if(lightServiceQueue == NULL){
        xReturned = pdFAIL;
    }

    /* USER CODE END */
    return xReturned;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while(1){
        light_event_t nextEvent;

        if(xQueueReceive(lightServiceQueue, &nextEvent, portMAX_DELAY) != pdFALSE){  // receive next item in queue
            nextEvent = NULL_EVENT;
        }

        switch (nextEvent) {
            case MEASURE_LIGHT:
                adcData_t lightDataBuf;
                // do measure light routine
                adcStartConversion(adcREG1, adcGROUP1);   // start conversion of ADC1 Group 1

                // check if conversion is complete and get data
                while(!adcIsConversionComplete(adcREG1, adcGROUP1));
                adcGetData(adcREG1, adcGROUP1, &lightDataBuf);


                // print ambient light value
                char s[MAX_STRING_SIZE];
                snprintf(s, MAX_STRING_SIZE, "Ambient Light %d", lightDataBuf.value);
                sciPrintText(scilinREG, (unsigned char *) s, strlen((char *) s));

                break;
            default:
                ;
        }
    }

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    xQueueSend(lightServiceQueue, event, portMAX_DELAY);
    /* USER CODE END */
    return 0;
}
