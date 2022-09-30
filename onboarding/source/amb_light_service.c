#include "amb_light_service.h"
#include "serial_io.h"
#include <stdio.h>

#include <adc.h>
#include <sci.h>
#include <FreeRTOS.h>
#include <os_task.h>

#include <os_queue.h>
#include <os_projdefs.h>
#include <string.h>

#define LIGHT_SENSOR 	6U

/* USER CODE BEGIN */
// Include any additional headers and global variables here

static TaskHandle_t lightServiceHandle = NULL;
static QueueHandle_t event_queue = NULL;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */

void adcStartConversion_selChn(adcBASE_t *adc, uint8_t channel, uint8_t fifo_size, uint8_t group);
void adcGetSingleData(adcBASE_t *adc, uint8_t group, adcData_t *data);
uint32_t getLightSensorData(void);
static void lightServiceTask(void * pvParameters);

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.
    BaseType_t xReturned = pdFAIL;
    
    if (event_queue == NULL) {
        event_queue = xQueueCreate(1, LIGHT_SERVICE_QUEUE_SIZE);
    }

    if (lightServiceHandle == NULL) {
        // Create controller task
        xReturned = xTaskCreate(lightServiceTask,             /* Function that implements the task. */
                                CONTROLLER_NAME_LIGHT,            /* Text name for the task. */
                                CONTROLLER_STACK_SIZE,      /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                CONTROLLER_PRIORITY,        /* Priority at which the task is created. */
                                &lightServiceHandle);     /* Used to pass out the created task's handle. */
    }

    if(xReturned == pdFAIL){
        unsigned char err[] = "Error Starting Light Service";
        sciPrintText(scilinREG, (unsigned char*) err, strlen((const char*) err));
    }

    if(event_queue == pdFAIL){
        unsigned char err[] = "Error Starting Queue";
        sciPrintText(scilinREG, (unsigned char*) err, strlen((const char*) err));
    }    
    /* USER CODE END */ 
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceHandle != NULL); 
    ASSERT(event_queue != NULL);
    light_event_t eventBuffer;

    uint8_t lightServiceStatus = initLightService();

    if(!lightServiceStatus) {
        unsigned char err[] = "Error Starting Light Service";
        sciPrintText(scilinREG, (unsigned char*) err, strlen((const char*) err));
    }

    while(true) {
        if(xQueueReceive(event_queue, &eventBuffer, ( TickType_t ) 0) == pdPASS) {
            if(eventBuffer == MEASURE_LIGHT) {
                uint16_t lightMeasurement = getLightSensorData();
                char output[7];
                sprintf(output, "%d", lightMeasurement);
                sciPrintText(sciREG, (unsigned char*) output, strlen((const char*) output));
            }
        }
    }

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    if(xQueueSend(event_queue, event, ( TickType_t ) 0) != pdPASS) {
        unsigned char err[] = "Error";
        sciPrintText(sciREG, (unsigned char*) err, strlen((const char*) err));
        return 0;   
    }
    /* USER CODE END */
    return 1;
}

void adcStartConversion_selChn(adcBASE_t *adc, uint8_t channel, uint8_t fifoSize, uint8_t group)
{
    adc->GxINTCR[group] = fifoSize;
    adc->GxSEL[group] = 1 << channel;
}

void adcGetSingleData(adcBASE_t *adc, uint8_t group, adcData_t *data)
{
    unsigned  buf;
    adcData_t *ptr = data; 

    buf        = adc->GxBUF[group].BUF0;
    ptr->value = (unsigned short)(buf & 0xFFFU);
    ptr->id    = (unsigned short)((buf >> 16U) & 0x1FU); // int to unsigned short

    adc->GxINTFLG[group] = 9U;
}

uint32_t getLightSensorData(void)
{
	adcData_t adcData;
	adcData_t *adcDataPtr = &adcData;

	adcStartConversion_selChn(adcREG1, LIGHT_SENSOR, 1, adcGROUP1);
 	while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 
	adcGetSingleData(adcREG1, adcGROUP1, adcDataPtr);
	
	return (adcDataPtr->value);
}

