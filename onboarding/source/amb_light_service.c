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

    if(xReturned == pdFAIL) {
        printf("Error - could not create light service task");
    }

    /* USER CODE END */

    
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    adcData_t adc_data;
    adcData_t *adc_data_ptr = &adc_data;

    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while (1) {
        if (xQueueReceive(xQueue, NULL, (TickType_t) 11) == pdPASS) {
            adcStartConversion(adcREG1, 
                                adcGROUP1);

            while (!adcIsConversionComplete(adcREG1, 
                                            adcGROUP1));

            adcGetData(adcREG1, 
                        adcGROUP1, 
                        adc_data_ptr);

            unsigned char text[2];

            // LSB by masking with 0xFF
            uint8_t first_half = (uint16_t) adc_data_ptr->value & 0xFF;
            uint8_t second_half = ((uint16_t) adc_data_ptr->value >> 8) & 0xFF;

            text[0] = (unsigned char) first_half;
            text[1] = (unsigned char) second_half;

            sciPrintText(scilinREG, 
                        (unsigned char*) text,
                        sizeof(text));
        }
    }
    
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    // don't need to dereference pointer to event as it takes ptr
    xQueueSend(xQueue, (light_event_t *) event, (TickType_t) 10);
    /* USER CODE END */
    return 0;
}
