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
#include <stdint.h>
#include <os_projdefs.h>
/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256UL
#define LIGHT_SERVICE_PRIORITY 1UL

/* USER CODE BEGIN */
// Define light service queue config here
#define QUEUE_LENGTH    10
#define ITEM_SIZE       sizeof( light_event_t )
/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here
static TaskHandle_t lightServiceTaskHandle = NULL;
static StaticTask_t lightServiceTaskBuffer;
static StackType_t lightServiceTaskStack[LIGHT_SERVICE_STACK_SIZE];
uint8_t lightServiceQueueStorage[ QUEUE_LENGTH * ITEM_SIZE ];
static StaticQueue_t lightServiceQueue;
static QueueHandle_t lightServiceQueueHandle = NULL;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here. Return error code if task/queue was not created successfully.
    if (lightServiceTaskHandle == NULL) 
    {
        // Create light service task
        lightServiceTaskHandle = xTaskCreateStatic( lightServiceTask,             /* Function that implements the task. */
                                                    LIGHT_SERVICE_NAME,            /* Text name for the task. */
                                                    LIGHT_SERVICE_STACK_SIZE,      /* Stack size in words, not bytes. */
                                                    NULL,                       /* Parameter passed into the task. */
                                                    LIGHT_SERVICE_PRIORITY,        /* Priority at which the task is created. */
                                                    lightServiceTaskStack,        /* Array to use as the task's stack. */
                                                    &lightServiceTaskBuffer);     /* Used to hold the task's data structure */
    }

    if (lightServiceTaskHandle == NULL)
        return OBC_ERR_CODE_TASK_CREATION_FAILED;
    
    if (lightServiceQueueHandle == NULL) 
    {
        lightServiceQueueHandle = xQueueCreateStatic ( QUEUE_LENGTH,
                                                       ITEM_SIZE,
                                                       lightServiceQueueStorage,
                                                       &lightServiceQueue);
    }

    if(lightServiceQueueHandle == NULL)
        return OBC_ERR_CODE_QUEUE_CREATION_FAILED;

    return OBC_ERR_CODE_SUCCESS;

    /* USER CODE END */
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    if (lightServiceQueueHandle != NULL)
    {
        light_event_t event;
        while (1)
        {
            if (xQueueReceive (lightServiceQueueHandle, 
                                &event,
                                ( TickType_t) 0) == pdTRUE && event == MEASURE_LIGHT)
            {
                adcStartConversion(adcREG1, adcGROUP1);
                if (adcIsConversionComplete(adcREG1, adcGROUP1) == 8) 
                {
                    adcData_t data;
                    adcGetData(adcREG1, adcGROUP1, &data);
                    sciPrintf("The light service data id: %lu, data: %u", data.id, data.value);
                }
            }

        }
        
    }
    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue. Return error code if event was not sent successfully.
    if (lightServiceQueueHandle != NULL) 
    {
        if (xQueueSend( lightServiceQueueHandle,
                        event,
                        ( TickType_t ) 0 ) != pdTRUE)
        {
            return OBC_ERR_CODE_QUEUE_SEND_FAILED;
        } 
        return OBC_ERR_CODE_SUCCESS;
    }
    return OBC_ERR_CODE_INVALID_ARG;
    
    /* USER CODE END */
}
