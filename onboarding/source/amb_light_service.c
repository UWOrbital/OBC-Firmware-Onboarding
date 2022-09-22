#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include <string.h>
#include <stdio.h>

#define LIGHT_SENSOR 6U

/* Declare handlers for light service task */
static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t lightServiceQueue = NULL;
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
        // Create light service task
        xReturned = xTaskCreate(lightServiceTask,
                                LIGHT_SERVICE_NAME,
                                LIGHT_SERVICE_STACK_SIZE,
                                NULL,
                                LIGHT_SERVICE_PRIORITY,
                                &lightServiceTaskHandle);
    }

    // Check if task was created successfully
    if (xReturned == pdFAIL) {
        return 0;
    }

    if (lightServiceQueue == NULL) {
        // Create light service queue
        lightServiceQueue = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH,
                                        LIGHT_SERVICE_QUEUE_ITEM_SIZE);
    }

    // Check if queue was created successfully
    if (lightServiceQueue == NULL) {
        return 0;
    }
    /* USER CODE END */
    return 1;
}

static void adcStartConversion_selChn(adcBASE_t *adc, uint32_t channel, uint32_t fifo_size, uint32_t group) {
    /** - Setup FiFo size */
    adc->GxINTCR[group] = fifo_size;

    /** - Start Conversion */
    adc->GxSEL[group] = 1 << channel;
}

static void adcGetSingleData(adcBASE_t *adc, uint32_t group, adcData_t *data) {
    uint32_t buf;
    adcData_t *ptr = data; 

    /** -  Get conversion data and channel/pin id */
    buf        = adc->GxBUF[group].BUF0;
    ptr->value = (uint16_t)(buf & 0xFFFU);
    ptr->id    = (uint16_t)((buf >> 16U) & 0x1FU); // int to unsigned short

    adc->GxINTFLG[group] = 9U;
}

static uint32_t getLightSensorData(void) {
	adcData_t adc_data;
	adcData_t *adc_data_ptr = &adc_data;

 	/** - Start Group1 ADC Conversion 
 	*     Select Channel 6 - Light Sensor for Conversion
 	*/
	adcStartConversion_selChn(adcREG1, LIGHT_SENSOR, 1, adcGROUP1);

 	/** - Wait for ADC Group1 conversion to complete */
 	while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 

	/** - Read the conversion result
	*     The data contains the Light sensor data
    */
	adcGetSingleData(adcREG1, adcGROUP1, adc_data_ptr);
	
	/** - Transmit the Conversion data to PC using SCI
    */
	return (adc_data_ptr->value);
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceTaskHandle != NULL);
    ASSERT(lightServiceQueue != NULL);

    light_event_t *light_event;
    BaseType_t returned = xQueueReceive(lightServiceQueue,
                                        &light_event,
                                        (TickType_t) 0);

    if (returned == pdPASS) {
        uint32_t sensor_data = getLightSensorData();
        char result[30];
        snprintf(result, 30, "Ambient light: %lu\n", sensor_data);
        sciPrintText(scilinREG, (unsigned char*) result, sizeof(result));
    }
    else {
        unsigned char* errorMsg = (unsigned char*) "Error receiving light event from queue.\n";
        sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
    }
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    BaseType_t returned = xQueueSend(lightServiceQueue, (void *) event, (TickType_t) 0);
    // Check if queue send was successful
    if (returned == pdPASS) {
        return 1;
    }
    
    /* USER CODE END */
    return 0;
}
