#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t lightServiceQueueHandle = NULL;

/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void *pvParameters);

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.
    BaseType_t xReturned;

    if (lightServiceTaskHandle == NULL) {
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                lightServiceTaskHandle);
    }

    if (lightServiceQueueHandle == NULL) {
        lightServiceQueueHandle = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH,
                                               LIGHT_SERVICE_QUEUE_ITEM_SIZE);
    }

    /* USER CODE END */
    return 0;
}

static void lightServiceTask(void *pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while (1) {
        light_event_t inMsg;

        if (lightServiceQueueHandle != NULL) {
            if (xQueueReceive(lightServiceQueueHandle,
                              inMsg,
                              LIGHT_SERVICE_QUEUE_WAIT_PERIOD) == pdPASS) {
                switch (inMsg)
                {
                case MEASURE_LIGHT:
                    /* read sensor measurement */
                       
                    /* send sensor measurement to serial console */
                    break;
                default:
                    break;
                }
            }
        }
    }

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    BaseType_t ret;

    ret = xQueueSend(lightServiceQueueHandle,
                     &event,
                     portMAX_DELAY);

    /* USER CODE END */
    return 0;
}
