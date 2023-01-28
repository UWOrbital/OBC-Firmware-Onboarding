#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <FreeRTOS.h>
#include <os_task.h>
#include <stdio.h>
#include <queue.h>
#include <os_queue.h>
#include <reg_adc.h>
#include <stdlib.h>

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
    if(lightServiceTaskHandle == NULL){
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                &lightServiceTaskHandle);

    }

    if(xReturned == pdFAIL){
        return 0;
    }

    xLightServiceQueue = xQueueCreate(LIGHT_SERVICE_QUEUE_SiZE, LIGHT_EVENT_SIZE);
    
    if(xLightServiceQueue == NULL){
        return 0;
    }
    /* USER CODE END */
    return 1;
}

uint16_t getLightSensorData(void)
{

	adcData_t lightData;
    	lightData.id = 6;
	adcData_t *adcDataPtr = &lightData;

 	/** - Start Group1 ADC Conversion 
 	*     Select Channel 6 - Light Sensor for Conversion
 	*/
	adcStartConversion(adcREG1, adcGROUP1);

 	/** - Wait for ADC Group1 conversion to complete */
 	while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 

	/** - Read the conversion result
	*     The data contains the Light sensor data
    */
	adcGetData(adcREG1, adcGROUP1, adc_data_ptr);
	
	/** - Transmit the Conversion data to PC using SCI
    */
	return (adcDataPtr->value);
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceTaskHandle != NULL);
    light_event_t queueTask = NULL_EVENT;
	
    while(1){

        if (xQueueReceive(xLightServiceQueue, &queueTask, portMAX_DELAY) == pdTRUE) {
            queueTask = NULL_EVENT;   
        }
        
        if (queueTask == MEASURE_LIGHT){
            
            uint16_t lightSensorData = getLightSensorData();
            char data[sizeof(lightSensorData)];
            int length = snprintf(data, sizeof(data), "%c", lightSensorData);

            sciPrintText(scilinREG, (unsigned char*)data, length + 1);
        }
        
    }
    /* USER CODE END */
}

/**
 * @brief Send a light service to the queue
 * @param light_event_t *event
 **/
uint8_t sendToLightServiceQueue(light_event_t *event) { 
    /* USER CODE BEGIN */
    // Send the event to the queue.
    if(event == NULL){
        
        return 0;
    }
    BaseType_t xReturned = xQueueSend(xLightServiceQueue, event, LIGHT_EVENT_SIZE);

    if(xReturned == pdPASS){

        return 1;

    }
    /* USER CODE END */
    return 0;
}
