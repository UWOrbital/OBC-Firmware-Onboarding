#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here
#include "FreeRTOS.h"
#include "os_task.h"
#include "os_queue.h"

#include <string.h>
#include <stdio.h>

void adcStartConversion_selChn(adcBASE_t *adc, uint8_t channel, uint8_t fifo_size, uint8_t group);
void adcGetSingleData(adcBASE_t *adc, uint8_t group, adcData_t *data);
static uint16_t getLightValue(void);

static TaskHandle_t lightServiceTaskHandle = NULL;
static QueueHandle_t lightServiceQueueHandle = NULL;
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
        // Create controller task
        xReturned = xTaskCreate(lightServiceTask,           /* Function that implements the task. */
                                LIGHT_SERVICE_NAME,         /* Text name for the task. */
                                LIGHT_SERVICE_STACK_SIZE,   /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                LIGHT_SERVICE_PRIORITY,     /* Priority at which the task is created. */
                                &lightServiceTaskHandle);   /* Used to pass out the created task's handle. */
    }

    

    if (lightServiceQueueHandle == NULL) {
        lightServiceQueueHandle = xQueueCreate(LIGHT_SERVICE_QUEUE_SIZE,
                                                LIGHT_SERVICE_ITEM_SIZE);

    }

    if(xReturned == pdFAIL ||
       lightServiceQueueHandle == NULL)
    {
        return 0;
    }
    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.
    ASSERT(lightServiceTaskHandle != NULL);
    ASSERT(lightServiceQueueHandle != NULL);
    
    light_event_t event;
    BaseType_t xReturned = pdFAIL;

    while(1) {
        xReturned = xQueueReceive(lightServiceQueueHandle,
                                &event,
                                0);
        if (xReturned == pdPASS && event == MEASURE_LIGHT) {
            char msg[20];
            uint16_t lightlevel = getLightValue();
            
            snprintf(msg, 20, "light level: %d\n", lightlevel);
            sciPrintText(scilinREG, (unsigned char*) msg, 20);

        }
    }
    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    return xQueueSend(lightServiceQueueHandle,
                    event,
                    0);
    /* USER CODE END */
    
}

void adcStartConversion_selChn(adcBASE_t *adc, uint8_t channel, uint8_t fifo_size, uint8_t group)
{
    /** - Setup FiFo size */
    adc->GxINTCR[group] = fifo_size;

    /** - Start Conversion */
    adc->GxSEL[group] = 1 << channel;
}

void adcGetSingleData(adcBASE_t *adc, uint8_t group, adcData_t *data)
{
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

uint16_t getLightValue(void){
    adcData_t lightData;

    adcStartConversion_selChn(adcREG1, LIGHT_SENSOR_PIN, 1, adcGROUP1);
    while(!adcIsConversionComplete(adcREG1, adcGROUP1));
    adcGetSingleData(adcREG1, adcGROUP1, &lightData);
    
    return lightData.value; 
}