#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers here
#include <FreeRTOS.h>
#include <os_projdefs.h>
#include <os_task.h>
#include <os_timer.h>
#include <sys_common.h>
#include <stdint.h>
#include <os_queue.h>
/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256UL
#define LIGHT_SERVICE_PRIORITY 1UL

/* USER CODE BEGIN */
// Define light service queue config here
#define LS_QUEUE_LENGTH 10
#define LS_ITEM_SIZE sizeof( light_event_t)
/* USER CODE END */

/* USER CODE BEGIN */
// Declare any global variables here
static TaskHandle_t lightServiceTaskHandle;
static StaticTask_t lightServiceTaskBuffer;
static StackType_t lightServiceTaskStack[LIGHT_SERVICE_STACK_SIZE];

static QueueHandle_t lightServiceQueueHandle = NULL;
static uint8_t lightServiceQueueStorage [LS_QUEUE_LENGTH*LS_ITEM_SIZE];
static StaticQueue_t lightServiceQueue;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here. Return error code if task/queue was not created successfully.
    if (lightServiceTaskHandle == NULL) {
        // Create controller task
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

    if (lightServiceQueueHandle == NULL){
        lightServiceQueueHandle = xQueueCreateStatic (LS_QUEUE_LENGTH,
                                                      LS_ITEM_SIZE,
                                                      lightServiceQueueStorage,
                                                      &lightServiceQueue  );
    }
    
    if(lightServiceQueueHandle == NULL)
        return OBC_ERR_CODE_QUEUE_CREATION_FAILED;

    return OBC_ERR_CODE_TASK_CREATION_FAILED;
    /* USER CODE END */
}


uint16_t Get_Light_Sensor_data(void)
{
    //Function taken from https://git.ti.com/cgit/hercules_examples/hercules_examples/tree/Launchpad/RM/RM46L8/Project_1/demoapp/source/adc_demos.c
	adcData_t adc_data;
	adcData_t *adc_data_ptr = &adc_data;

 	/** - Start Group1 ADC Conversion 
 	*     Select Channel 6 - Light Sensor for Conversion
 	*/
	adcStartConversion(adcREG1, adcGROUP1);

 	/** - Wait for ADC Group1 conversion to complete */
 	while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 

	/** - Read the conversion result
	*     The data contains the Light sensor data
    */
	adcGetData(adcREG1, adcGROUP1, adc_data_ptr);
	
	/** - Transmit the Conversion data to PC using SCI
    */
	return (adc_data_ptr->value);
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    light_event_t LightEvent;
    uint16_t lightdata = Get_Light_Sensor_data();
    if (xQueueReceive(lightServiceQueueHandle, &LightEvent, 10) == pdPASS && LightEvent == MEASURE_LIGHT){
        sciPrintf("Ambient light value: ", lightdata);
    }
    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue. Return error code if event was not sent successfully.
    if (xQueueSend(lightServiceQueueHandle, event, 10)==pdPASS){
        return OBC_ERR_CODE_SUCCESS;
    }

    if (event == NULL){
        return OBC_ERR_CODE_INVALID_ARG;
    }

    return OBC_ERR_CODE_QUEUE_FULL;
    /* USER CODE END */
}
