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
#define QUEUE_LENGTH 1   
#define QUEUE_ITEM_SIZE sizeof(light_event_t)

/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here
static TaskHandle_t lightServiceTaskHandle;
static StaticTask_t lightServiceTaskBuffer;
static StackType_t lightServiceTaskStack[LIGHT_SERVICE_STACK_SIZE];

static QueueHandle_t eventQueueHandle;
static uint8_t eventQueueStorageBuffer[QUEUE_LENGTH*QUEUE_ITEM_SIZE];
static StaticQueue_t eventQueueBuffer;

/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here. Return error code if task/queue was not created successfully.
    lightServiceTaskHandle = xTaskCreateStatic( lightServiceTask,
                                                LIGHT_SERVICE_NAME,
                                                LIGHT_SERVICE_STACK_SIZE,
                                                NULL,
                                                LIGHT_SERVICE_PRIORITY,
                                                lightServiceTaskStack,
                                                &lightServiceTaskBuffer);
    
    if (lightServiceTaskHandle == NULL) 
        return OBC_ERR_CODE_TASK_CREATION_FAILED;

    eventQueueHandle = xQueueCreateStatic(  QUEUE_LENGTH,
                                            QUEUE_ITEM_SIZE,
                                            eventQueueStorageBuffer,
                                            &eventQueueBuffer);
    
    if (eventQueueHandle == NULL)
        return OBC_ERR_CODE_QUEUE_CREATION_FAILED;

    return OBC_ERR_CODE_SUCCESS;
    /* USER CODE END */
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceTaskHandle != NULL);

    light_event_t lightEventBuffer;

    BaseType_t xCheckEventReturned = xQueueReceive( eventQueueHandle, 
                                                    &lightEventBuffer, 
                                                    0);
    while (1) {
        if (xCheckEventReturned == pdTRUE) {
            // Measure the light and print the result
            adcData_t adcData;

            // Start ADC1 Group 1 conversion
            adcStartConversion(adcREG1, adcGROUP1);

            // Wait for conversion to complete
            while (!adcIsConversionComplete(adcREG1, adcGROUP1));

            // Read conversion result data into adcData
            adcGetData(adcREG1, adcGROUP1, &adcData);

            sciPrintf("%d\n", adcDataPtr->value);
        }
    }

    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    if (event == NULL) {
        return OBC_ERR_CODE_INVALID_ARG;
    }

    // Send the event to the queue. Return error code if event was not sent successfully.
    BaseType_t xSendEventRetVal = xQueueSend(eventQueueHandle, event, 0);

    if (xSendEventRetVal == errQUEUE_FULL) 
        return OBC_ERR_CODE_QUEUE_FULL;    
    
    return OBC_ERR_CODE_SUCCESS;
    /* USER CODE END */
}
