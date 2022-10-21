#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <stdio.h>
#include <math.h>
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t lightQueueHandle = NULL;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.
    if (lightServiceTaskHandle == NULL) {
        BaseType_t xReturned = xTaskCreate(lightServiceTask,
                                            LIGHT_SERVICE_NAME,
                                            LIGHT_SERVICE_STACK_SIZE,
                                            NULL,
                                            LIGHT_SERVICE_PRIORITY,
                                            &lightServiceTaskHandle);
        if (xReturned == pdFAIL) {
            sciPrintText(scilinREG, (unsigned char *) "Error: Cannot start light service task", 38);
            return 0;
        }
    }

    if (lightQueueHandle == NULL) {
        lightQueueHandle = xQueueCreate(QUEUE_SIZE, QUEUE_ITEM_SIZE);
        if (lightQueueHandle == NULL) {
            sciPrintText(scilinREG, (unsigned char *) "Error: Cannot initialize light queue", 36);
            return 0;
        }
    }
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    if (lightQueueHandle != NULL) {
        light_event_t event;
        if (xQueueReceive(lightQueueHandle, &event, 1000) == pdPASS) {
            if (event == MEASURE_LIGHT) {
                adcData_t data;
                adcData_t * dataPtr = &data;
                adcStartConversion(adcREG1, adcGROUP1);
                while (!adcIsConversionComplete(adcREG1, adcGROUP1));
                adcGetData(adcREG1, adcGROUP1, dataPtr);

                uint16 value = dataPtr->value;
                uint32_t outputLength = (uint32_t) log10(value);
                char output[outputLength + 1];
                snprintf(output, outputLength, "%d", value);
                sciPrintText(scilinREG, (unsigned char *) output, outputLength);
            }
        }
    }
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    if (lightQueueHandle != NULL) {
        if (xQueueSend(lightQueueHandle, MEASURE_LIGHT, 1000) != pdPASS) {
            sciPrintText(scilinREG, (unsigned char *) "Error: Unable to send MEASURE_LIGHT event", 41);
            return 1;
        }
    }
    /* USER CODE END */
    return 0;
}
