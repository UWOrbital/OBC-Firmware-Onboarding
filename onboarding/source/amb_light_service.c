#include "amb_light_service.h"
#include "serial_io.h"
//#include <hal/include/adc.h>

#include <stdio.h>
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
static TaskHandle_t lightServiceHandle = NULL;
static QueueHandle_t lightServiceQueue;

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

    // Create light task
    if (lightServiceHandle == NULL) {
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                &lightServiceHandle);
    }
    
    if (xReturned == pdFAIL) {
            printf("\nlightServiceTask not created...\n");
    }

    // Create queue
    if (lightServiceQueue == NULL) {
        lightServiceQueue = xQueueCreate(LIGHT_QUEUE_LENGTH, LIGHT_QUEUE_SIZE);

        if (lightServiceQueue == NULL) {
            sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
        }
    }

    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    adcData_t adcData;
    light_event_t eventReceived;

    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while (1) {
        if (xQueueReceive(lightServiceQueue, &eventReceived, portMAX_DELAY) == pdPASS) {

            if (eventReceived == MEASURE_LIGHT) {
                adcStartConversion(adcREG1, adcGROUP1);
                while (!adcIsConversionComplete(adcREG1, adcGROUP1));
                adcGetData(adcREG1, adcGROUP1, &adcData);

                char txtBuffer[TEXT_SIZE];
                int count = snprintf(txtBuffer, TEXT_SIZE, "%u\r\n", adcData.value); 
                sciPrintText(scilinREG, (unsigned char*)txtBuffer, count);
            }

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
