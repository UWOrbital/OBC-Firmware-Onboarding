#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <sys_common.h>

static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t xLightServiceQueue = NULL;
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
    if (lightServiceTaskHandle == NULL){
        xReturned = xTaskCreate(LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                &lightServiceTaskHandle);
    }
    if(xLightServiceQueue == NULL){
        xLightServiceQueue = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH,
                                          LIGHT_SERVICE_QUEUE_ITEM_SIZE);
    }
    if(xLightServiceQueue == NULL){
        xReturned = pdFAIL;
    }

    return xReturned;
    /* USER CODE END */
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    light_event_t buffer;
    if(xQueueReceive(xLightServiceQueue, &buffer, portMAX_DELAY) == pdTRUE){
        adcData_t adc_data;
        adcData_t *adc_data_ptr = &adc_data;

        adcStartConversion_selChn(adcREG1, Light_Sensor, 1, adcGROUP1);

        while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 

        adcGetSingleData(adcREG1, adcGROUP1, adc_data_ptr);

        char str[MAX_STRING_SIZE];
        snprintf(str, MAX_STRING_SIZE, "The ambient light is %d", adc_data_ptr->value);

        sciPrintText(scilinREG, (unsigned char *) str, strlen(str));
    }

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    BaseType_t xReturned = pdFAIL;
    if(xLightServiceQueue != NULL){
        xReturned = xQueueSend(xLightServiceQueue, event, portMAX_DELAY);
    }
    return xReturned;
    /* USER CODE END */
}
