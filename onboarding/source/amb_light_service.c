#include "amb_light_service.h"
#include "serial_io.h"

#include <FreeRTOS.h>

#include <os_task.h>
#include <os_queue.h>

#include <sci.h>
#include <adc.h>

#include <stdio.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
static TaskHandle_t xLightServiceHandle = NULL;
static QueueHandle_t xQueue = NULL;
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

    if (xLightServiceHandle == NULL) {
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                &xLightServiceHandle
                            );
    }

    // Create queue
    if (xQueue == NULL) {
        xQueue = xQueueCreate(LIGHT_QUEUE_LENGTH, 
                        LIGHT_SERVICE_ITEM_SIZE);
    }

    if (xQueue == NULL || xReturned == pdFAIL) {
        sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
    }

    /* USER CODE END */

    
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    adcData_t adcData;
    light_event_t *currentEvent = NULL;

    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while (1) {
        if (xQueueReceive(xQueue, currentEvent, portMAX_DELAY) == pdPASS) {
            
            if(currentEvent == MEASURE_LIGHT) {
                adcStartConversion(adcREG1, 
                                    adcGROUP1);

                while (!adcIsConversionComplete(adcREG1, 
                                                adcGROUP1));

                adcGetData(adcREG1, 
                            adcGROUP1, 
                            &adcData);

                char text[TEXT_SIZE];

                int count = snprintf(text, TEXT_SIZE, "%u\r\n", adcData.value);

                if(count <= 0) {
                    sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
                }

                sciPrintText(scilinREG, 
                            (unsigned char*) text,
                            count);
            }
        }
    }
    
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    xQueueSend(xQueue, event, portMAX_DELAY);
    /* USER CODE END */
    return 0;
}
