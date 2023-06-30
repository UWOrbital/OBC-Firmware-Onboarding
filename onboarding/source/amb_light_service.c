#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers here
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256UL
#define LIGHT_SERVICE_PRIORITY 1UL

/* USER CODE BEGIN */
// Define light service queue config here
#define SERVICE_QUEUE_LENGTH 1
#define SERVICE_QUEUE_ITEM_SIZE sizeof(light_event_t)
/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here
static TaskHandle_t lightTaskHandle;
static StaticTask_t lightTaskBufferr;
static StackType_t  lightTaskStack[LIGHT_SERVICE_STACK_SIZE];

static StaticQueue_t xStaticQueueBuffer;
static QueueHandle_t eventQueueHandle; 
static uint8_t ucQueueStorageArea[ SERVICE_QUEUE_LENGTH * SERVICE_QUEUE_ITEM_SIZE];
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here. Return error code if task/queue was not created successfully.
    if (lightTaskHandle == NULL){
        lightTaskHandle = xTaskCreateStatic( lightServiceTask,
                                             LIGHT_SERVICE_NAME,
                                             LIGHT_SERVICE_STACK_SIZE,
                                             NULL,
                                             LIGHT_SERVICE_PRIORITY,
                                             lightTaskStack,
                                             &lightTaskBufferr);
        
    }

    if (lightTaskHandle == NULL){
        return OBC_ERR_CODE_TASK_CREATION_FAILED;
    }

    eventQueueHandle = xQueueCreateStatic( SERVICE_QUEUE_LENGTH,
                                           SERVICE_QUEUE_ITEM_SIZE,
                                           ucQueueStorageArea,
                                           &xStaticQueueBuffer);

    if (eventQueueHandle == NULL){
        return OBC_ERR_CODE_QUEUE_CREATION_FAILED;
    }

    return OBC_ERR_CODE_SUCCESS;
    /* USER CODE END */
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    if (eventQueueHandle != NULL){
        light_event_t detectedLightEvent;

        if(xQueueReceive( eventQueueHandle, &detectedLightEvent, portMAX_DELAY) == pdTRUE){
            
            adcData_t adcData;
            adcStartConversion(adcREG1, adcGROUP1);

            while(!adcIsConversionComplete(adcREG1, adcGROUP1));

            adcGetData(adcREG1, adcGROUP1, &adcData);
            sciPrintf("%d\n", adcData);
        }
    }

    while(1); // i
    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue. Return error code if event was not sent successfully.
    BaseType_t messageStatus;
    
    messageStatus = (eventQueueHandle, event,portMAX_DELAY);

    if (messageStatus){
        return OBC_ERR_CODE_SUCCESS;
    }

    return OBC_ERR_CODE_QUEUE_FULL;
    /* USER CODE END */
}
