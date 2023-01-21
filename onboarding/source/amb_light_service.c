#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <FreeRTOS.h>
#include <os_task.h>
#include <stdio.h>
#include <queue.h>
#include <os_queue.h>
#include <reg_adc.h>
#include <stdlib.h>

static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t xLightServiceQueue = NULL;
light_event_t light_event;
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
    if(lightServiceTaskHandle == NULL){
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                &lightServiceTaskHandle);

    }

    if(xLightServiceQueue == NULL){
        xLightServiceQueue = xQueueCreate(QUEUE_SiZE, LIGHT_EVENT_SIZE);
    }
    /* USER CODE END */
    return 1;
}

/**
 * @brief Get light data from adcData
 * @param void
 **/
uint8_t* Get_Light_Sensor_data(void)
{

	adcData_t *adc_data_ptr = pvPortMalloc(sizeof(adcData_t));
    adc_data_ptr->id = 6;
    //adc_data_ptr->value;

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
    ASSERT(lightServiceTaskHandle != NULL);
    uint8_t lightServiceStatus = initLightService();
    
    if(lightServiceStatus == 0){
        
        printf("Failed to create a queue");
        
    }else{
        
        uint32 Light_Sensor_Data = Get_Light_Sensor_data();
        int length = snprintf(NULL, 0, "&#37;u", Light_Sensor_Data);
        char *data = malloc(length + 1);
        
        snprintf(data, "&#37;u", Light_Sensor_Data);
        sciPrintText(scilinREG, data, length + 1);
        
        free(data);
    }
    /* USER CODE END */
}

/**
 * @brief Send a light service to the queue
 * @param light_event_t *event
 **/
uint8_t sendToLightServiceQueue(light_event_t *event) { 
    /* USER CODE BEGIN */
    // Send the event to the queue.
    event = &light_event;
    lightServiceTask(event);
    /* USER CODE END */
    return 0;
}
