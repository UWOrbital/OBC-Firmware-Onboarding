#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"


#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <stdint.h>


// Include any additional headers here

/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256UL
#define LIGHT_SERVICE_PRIORITY 1UL

/* USER CODE BEGIN */
// Define light service queue config here
#define LIGHT_QUEUE_LENGTH 10
#define LIGHT_QUEUE_ITEM_SIZE sizeof(light_event_t)
/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here


static TaskHandle_t lightServiceHandle = NULL;
static StaticTask_t lightServiceBuffer;
static StackType_t lightServiceStack[LIGHT_SERVICE_STACK_SIZE];

static QueueHandle_t lightserviceQueue;
static StaticQueue_t xlightStaticQueue;
uint8_t uclightQueueStorageArea[LIGHT_QUEUE_LENGTH * LIGHT_QUEUE_ITEM_SIZE];

/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
    if (lightServiceHandle == NULL) {
        lightServiceHandle = xTaskCreateStatic(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                lightServiceStack,
                                &lightServiceBuffer);
                                    }
    if (lightServiceHandle == NULL)
    {
        return OBC_ERR_CODE_TASK_CREATION_FAILED;
    }

    if (lightserviceQueue == NULL) {
        lightserviceQueue = xQueueCreateStatic(LIGHT_QUEUE_LENGTH, LIGHT_QUEUE_ITEM_SIZE,uclightQueueStorageArea, &xlightStaticQueue);
    }
    if (lightserviceQueue == NULL) 
        return OBC_ERR_CODE_QUEUE_CREATION_FAILED;
    
    
return OBC_ERR_CODE_SUCCESS;     
}                      
/* USER CODE END */


uint16_t getLightSensorData(void)
{
	adcData_t adcData;
	

 	/** - Start Group1 ADC Conversion 
 	*     Select Channel 6 - Light Sensor for Conversion
 	*/
	adcStartConversion(adcREG1, adcGROUP1);

 	/** - Wait for ADC Group1 conversion to complete */
 	while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 

	/** - Read the conversion result
	*     The data contains the Light sensor data
    */
	adcGetData(adcREG1, adcGROUP1, &adcData);
	
	return adcData.value;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    light_event_t eventReceived;

    while (1) {
        if (xQueueReceive(lightserviceQueue, &eventReceived, portMAX_DELAY) == pdPASS) {
            
            if (eventReceived == MEASURE_LIGHT) {
                

                uint16_t lightdata = getLightSensorData();
                    /** - Transmit the Conversion data to PC using SCI*/
                    sciPrintf("The ambient light is %u\n", lightdata);
             }

                
          }
    }
    /* USER CODE END */
 }

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    if(lightserviceQueue != NULL){
        if(xQueueSend(lightserviceQueue, event, portMAX_DELAY == pdTRUE)){
            return OBC_ERR_CODE_SUCCESS;
        }
        return OBC_ERR_CODE_QUEUE_FULL;
    }
    return OBC_ERR_CODE_INVALID_STATE;
    /* USER CODE END */
}
