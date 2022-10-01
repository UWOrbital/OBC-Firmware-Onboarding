#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include <os_task.h>
#include <FreeRTOS.h>

#define Light_Sensor 6U
static TaskHandle_t lightServiceTaskHandle = NULL;

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
    if(lightServiceTaskHandle == NULL) {
        // Create controller task
        xReturned = xTaskCreate(lightServiceTask,             /* Function that implements the task. */
                                LIGHT_SERVICE_NAME,            /* Text name for the task. */
                                LIGHT_SERVICE_STACK_SIZE,      /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                LIGHT_SERVICE_PRIORITY,        /* Priority at which the task is created. */
                                &lightServiceTaskHandle);     /* Used to pass out the created task's handle. */
    }

    event_queue = xQueueCreate(event_queue_len, sizeof(MEASURE_LIGHT));

    if(xReturned == pdFAIL) {
        // error: couldnt create light service task
    }
    if(event_queue == NULL) {
        // error: couldnt create event queue
    }
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    light_event_t ignore = 0;
    if(xQueueReceive(event_queue, (void *)&ignore, 0) == pdTRUE) {
        // new event received
        uint32 light_sensor_data = getLightSensorData();
        unsigned char ambient_light[10] = "";
        sprintf(ambient_light, "%d", (long)light_sensor_data);
        sciPrintText(scilinREG, (unsigned char *)ambient_light, strlen(ambient_light));
    }
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    if(xQueueSend(event_queue, (void *)event, 0) != pdTRUE) {
        char error_message [] = "Could not send to queue!";
        sciPrintText(scilinREG, (unsigned char *)error_message, strlen(error_message));
    }
    /* USER CODE END */
    return 0;
}

void adcStartConversion_selChn(adcBASE_t *adc, unsigned channel, unsigned fifo_size, unsigned group)
{
    /** - Setup FiFo size */
    adc->GxINTCR[group] = fifo_size;
    /** - Start Conversion */
    adc->GxSEL[group] = 1 << channel;
}

void adcGetSingleData(adcBASE_t *adc, unsigned group, adcData_t *data) {
    unsigned  buf;
    adcData_t *ptr = data; 

    /** -  Get conversion data and channel/pin id */
    buf        = adc->GxBUF[group].BUF0;
    ptr->value = (unsigned short)(buf & 0xFFFU);
    ptr->id    = (unsigned short)((buf >> 16U) & 0x1FU); // int to unsigned short

    adc->GxINTFLG[group] = 9U;

    /**   @note The function canInit has to be called before this function can be used.\n
    *           The user is responsible to initialize the message box.
    */
}

uint32 getLightSensorData(void) {
	adcData_t adc_data;
	adcData_t *adc_data_ptr = &adc_data;

 	/** - Start Group1 ADC Conversion 
 	*     Select Channel 6 - Light Sensor for Conversion
 	*/
	adcStartConversion(adcREG1, Light_Sensor, 1, adcGROUP1);

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