#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers here
#include <FreeRTOS.h>
#include <os_queue.h>
#include <stdint.h>
#include <os_projdefs.h>
#include <os_task.h>
/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256UL
#define LIGHT_SERVICE_PRIORITY 1UL

/* USER CODE BEGIN */
// Define light service queue config here
#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(light_event_t)

/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here
static TaskHandle_t lightServiceTaskHandle = NULL;
static StaticTask_t lightServiceTaskBuffer;
static StackType_t lightServiceTaskStack[LIGHT_SERVICE_STACK_SIZE];

static QueueHandle_t queueHandle = NULL;
static StaticQueue_t queueBuffer;
uint8_t ucQueueStorageArea[QUEUE_LENGTH * ITEM_SIZE];
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void *pvParameters);

obc_error_code_t initLightService(void)
{
    /* USER CODE BEGIN */
    // Create the task and queue here. Return error code if task/queue was not created successfully.
    if (lightServiceTaskHandle == NULL)
    {
        lightServiceTaskHandle = xTaskCreateStatic(lightServiceTask,
                                                   LIGHT_SERVICE_NAME,
                                                   LIGHT_SERVICE_STACK_SIZE,
                                                   NULL,
                                                   LIGHT_SERVICE_PRIORITY,
                                                   lightServiceTaskStack,
                                                   &lightServiceTaskBuffer);
    }

    if (lightServiceTaskHandle == NULL)
    {
        return OBC_ERR_CODE_TASK_CREATION_FAILED;
    }

    if (queueHandle == NULL)
    {
        queueHandle = xQueueCreateStatic(QUEUE_LENGTH,
                                         ITEM_SIZE,
                                         ucQueueStorageArea,
                                         &queueBuffer);
    }

    if (queueHandle == NULL)
    {
        return OBC_ERR_CODE_QUEUE_CREATION_FAILED;
    }

    return OBC_ERR_CODE_SUCCESS;
    /* USER CODE END */
}

uint16_t getLightSensorData(void)
{
    adcData_t adc_data;

    /** - Wait for ADC Group1 conversion to complete */

    adcStartConversion(adcREG1, adcGROUP1);
    /** - Read the conversion result
     *     The data contains the Light sensor data
     */
    while (!adcIsConversionComplete(adcREG1, adcGROUP1))
        ;
    adcGetData(adcREG1, adcGROUP1, &adc_data);
    return adc_data.value;
}

static void lightServiceTask(void *pvParameters)
{
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    if (queueHandle != NULL)
    {
        light_event_t lightEvent;
        while (1)
        {
            // queueBuffer has different type with event, will it convert automatically?
            if (xQueueReceive(queueHandle, &lightEvent, (TickType_t)0) == pdTRUE)
            {
                if (lightEvent == MEASURE_LIGHT)
                {
                    uint16_t lightData = getLightSensorData();
                    // use %lu if uint32_t (long unsigned int)
                    sciPrintf("%u\n", lightData);
                };
            }
        }
    }

    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event)
{
    /* USER CODE BEGIN */
    // Send the event to the queue. Return error code if event was not sent successfully.
    if (queueHandle != NULL)
    {
        if (xQueueSend(queueHandle, event, (TickType_t)0) == pdTRUE)
        {
            // Are there any other issue that may cause queuesend fail?
            // should I check if the queue is full?
            return OBC_ERR_CODE_SUCCESS;
        }
    }
    return OBC_ERR_CODE_QUEUE_SEND_FAILED;
    /* USER CODE END */
}
