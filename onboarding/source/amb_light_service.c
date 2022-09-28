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

#define Light_Sensor 	6U

/* USER CODE BEGIN */
// Include any additional headers and global variables here

static TaskHandle_t lightServiceHandle = NULL;
static QueueHandle_t queue = NULL;
/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */

void adcStartConversion_selChn(adcBASE_t *adc, unsigned channel, unsigned fifo_size, unsigned group);
void adcGetSingleData(adcBASE_t *adc, unsigned group, adcData_t *data);
uint32 Get_Light_Sensor_data(void);
static void lightServiceTask(void * pvParameters);
uint8_t sendToLightServiceQueue(light_event_t *event);

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.
    BaseType_t xReturned = pdFAIL;
    
    if (queue == NULL) {
        queue = xQueueCreate(1, sizeof(MEASURE_LIGHT));
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
        sciPrintText(sciREG, (unsigned char*) err, strlen((const char*) err));
    }
    /* USER CODE END */ 
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceHandle != NULL); 
    ASSERT(queue != NULL);
    light_event_t eventBuffer;

    uint8_t lightServiceStatus = initLightService();

    while(true) {
        if(xQueueReceive(queue, &eventBuffer, ( TickType_t ) 0) == pdPASS) {
            if(eventBuffer == MEASURE_LIGHT) {
                uint16_t lightMeasurement = Get_Light_Sensor_data();
                char output[6];
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
    if(xQueueSend(queue, &event, ( TickType_t ) 0) != pdPASS) {
        unsigned char err[] = "Error";
        sciPrintText(sciREG, (unsigned char*) err, strlen((const char*) err));
        return 1;   
    }
    /* USER CODE END */
    return 0;
}

void adcStartConversion_selChn(adcBASE_t *adc, unsigned channel, unsigned fifo_size, unsigned group)
{
    adc->GxINTCR[group] = fifo_size;
    adc->GxSEL[group] = 1 << channel;
}

void adcGetSingleData(adcBASE_t *adc, unsigned group, adcData_t *data)
{
    unsigned  buf;
    adcData_t *ptr = data; 

    buf        = adc->GxBUF[group].BUF0;
    ptr->value = (unsigned short)(buf & 0xFFFU);
    ptr->id    = (unsigned short)((buf >> 16U) & 0x1FU); // int to unsigned short

    adc->GxINTFLG[group] = 9U;
}

uint32 Get_Light_Sensor_data(void)
{
	adcData_t adc_data;
	adcData_t *adc_data_ptr = &adc_data;

	adcStartConversion_selChn(adcREG1, Light_Sensor, 1, adcGROUP1);
 	while(!adcIsConversionComplete(adcREG1, adcGROUP1)); 
	adcGetSingleData(adcREG1, adcGROUP1, adc_data_ptr);
	
	return (adc_data_ptr->value);
}

