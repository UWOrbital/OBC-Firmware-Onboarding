#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t xQueue1 = NULL; 


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
    if (lightServiceTaskHandle = NULL) {
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                (void *) 0,
                                LIGHT_SERVICE_PRIORITY,
                                &lightServiceTaskHandle);
    if (xQueue1 = NULL) {
        xQueue1 = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    }
    }
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceTaskHandle != NULL);

    light_event_t *event;
    while(1) {
        if (xQueueReceive(xQueue1, (void *) event, (TickType_t) 0) == pdPASS) {
            
        }
    }

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    if (xQueueSend(xQueue1, (void *) event, (TickType_t) 0) == pdPASS) {
        return 1;
    }
    /* USER CODE END */
    return 0;
}

// Function that does ADC conversion and returns ambient light value
uint32_t getAmbientLightData(void);
