#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include <stdio.h>
#include <string.h>

static QueueHandle_t lightServiceQueueHandle = NULL;
static TaskHandle_t lightServiceTaskHandle = NULL;

static void measureLight(void);
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
    if (lightServiceTaskHandle == NULL) {
        xReturned = xTaskCreate(lightServiceTask,             /* Function that implements the task. */
                                LIGHT_SERVICE_NAME,            /* Text name for the task. */
                                LIGHT_SERVICE_STACK_SIZE,      /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                LIGHT_SERVICE_PRIORITY,        /* Priority at which the task is created. */
                                &lightServiceTaskHandle);     /* Used to pass out the created task's handle. */
    }

    if (lightServiceQueueHandle == NULL) {
        lightServiceQueueHandle = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH, LIGHT_SERVICE_QUEUE_ITEM_SIZE);
    }

    if ( (xReturned != pdPASS) || (lightServiceQueueHandle == NULL) ) {
        return 0;
    }
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    while (1) {
        light_event_t event;
        if (xQueueReceive(lightServiceQueueHandle, (void *)&event, portMAX_DELAY)) {
            switch(event) {
                case MEASURE_LIGHT:
                    measureLight();
                    break;
            }
        }
    }
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    if (xQueueSend(lightServiceQueueHandle, (void *)event, portMAX_DELAY) == pdPASS) {
        return 1;
    }
    /* USER CODE END */
    return 0;
}

static void measureLight(void) {
    adcData_t adc_data[1];

    adcStartConversion(adcREG1, adcGROUP1);

    /* wait and read the conversion count */
    while((adcIsConversionComplete(adcREG1, adcGROUP1)) == 0);
    adcGetData(adcREG1, adcGROUP1, adc_data);
    
    /* conversion results: */
    /* adc_data[0] -> should have conversions for Group1 channel 6 */

    char text[38] = {0};
    snprintf(text, 33, "Ambient Light ADC Value: %d\r\n", (int)adc_data[0].value);
    sciPrintText(scilinREG, (unsigned char *)text, 38);
}