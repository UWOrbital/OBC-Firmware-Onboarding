#include "amb_light_service.h"
#include "serial_io.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
static TaskHandle_t lightServiceHandle = NULL;
static QueueHandle_t xQueue;

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

    xReturned = xTaskCreate(lightServiceTask,
                            LIGHT_SERVICE_NAME,
                            LIGHT_SERVICE_STACK_SIZE,
                            NULL,
                            LIGHT_SERVICE_PRIORITY,
                            &lightServiceHandle);

    // Create queue
    xQueue = xQueueCreate(LIGHT_QUEUE_LENGTH, LIGHT_QUEUE_LENGTH);
    
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while(xQueueReceive(xQueue, NULL, (TickType_t) 10) == pdFAIL);

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    xQueueSend(xQueue, (void *) event, (TickType_t) 10);
    /* USER CODE END */
    return 0;
}
