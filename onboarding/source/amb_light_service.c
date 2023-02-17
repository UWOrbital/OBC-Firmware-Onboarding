#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"


#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
<<<<<<< HEAD
#include <string.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <adc.h>
#include <sci.h>
=======
// Include any additional headers here

/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256UL
#define LIGHT_SERVICE_PRIORITY 1UL

/* USER CODE BEGIN */
// Define light service queue config here

/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d

static TaskHandle_t lightServiceHandle = NULL;
static QueueHandle_t lightserviceQueue;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
<<<<<<< HEAD
    BaseType_t xReturned = pdFAIL;
        if (lightServiceHandle == NULL) {
            xReturned = xTaskCreate(lightServiceTask,
                                    LIGHT_SERVICE_NAME,
                                    LIGHT_SERVICE_STACK_SIZE,
                                    NULL,
                                    LIGHT_SERVICE_PRIORITY,
                                    &lightServiceHandle);
    
    /* USER CODE END */
        }
        if (xReturned == pdFAIL) {
             printf("\nlightServiceTask not created...\n");
        
        }

        if (lightserviceQueue == NULL) {
                lightserviceQueue = xQueueCreate(LIGHT_SERVICE_SIZE, LIGHT_QUEUE_SIZE);

                if (lightserviceQueue == NULL) {
                     sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
                }
        }
    return 1;
=======
    // Create the task and queue here. Return error code if task/queue was not created successfully.

    /* USER CODE END */
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    light_event_t eventReceived;


 	while (1) {
        if (xQueueReceive(lightserviceQueue, &eventReceived, portMAX_DELAY) == pdPASS) {
            adcData_t adcData;
            if (eventReceived == MEASURE_LIGHT) {
                adcStartConversion(adcREG1, adcGROUP1);
                while (!adcIsConversionComplete(adcREG1, adcGROUP1));
                adcGetData(adcREG1, adcGROUP1, &adcData);

                char str[32];
                    snprintf(str, 32, "The ambient light is %d", adcData.value);
                    sciPrintText(scilinREG, (unsigned char *) str, strlen(str));
                }

                
            }
    }
    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
<<<<<<< HEAD
     xQueueSend(lightserviceQueue, event, portMAX_DELAY);

    
=======
    // Send the event to the queue. Return error code if event was not sent successfully.
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d
    
    /* USER CODE END */
}
