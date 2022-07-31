#include "amb_light_service.h"
#include "serial_io.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include <adc.h>
#include <sci.h>

#include <stdio.h>
#include <string.h>

QueueHandle_t lightServiceQueueHandle = NULL;
TaskHandle_t lightServiceTaskHandle = NULL;

static void lightServiceTask(void * pvParameters);

uint8_t initLightService(void) {
    BaseType_t xReturned;
    xReturned = xTaskCreate(lightServiceTask,             /* Function that implements the task. */
                            LIGHT_SERVICE_NAME,            /* Text name for the task. */
                            LIGHT_SERVICE_STACK_SIZE,      /* Stack size in words, not bytes. */
                            NULL,                       /* Parameter passed into the task. */
                            LIGHT_SERVICE_PRIORITY,        /* Priority at which the task is created. */
                            &lightServiceTaskHandle);     /* Used to pass out the created task's handle. */

    lightServiceQueueHandle = xQueueCreate(LIGHT_SERVICE_QUEUE_LENGTH, LIGHT_SERVICE_QUEUE_ITEM_SIZE);

    if (xReturned != pdPASS) {
        return 0;
    }
    if (lightServiceQueueHandle == NULL) {
        return 0;
    }

    return 1;
}

static void lightServiceTask(void * pvParameters) {
    while(1) {
        adcData_t adc_data[1];
        light_event_t event;
        
        if (xQueueReceive(lightServiceQueueHandle, (void *)&event, portMAX_DELAY)) {
            switch(event) {
                case MEASURE_LIGHT:
                    adcStartConversion(adcREG1, adcGROUP1);

                    /* wait and read the conversion count */
                    while((adcIsConversionComplete(adcREG1,adcGROUP1)) == 0);
                    adcGetData(adcREG1, adcGROUP1, adc_data);
                    
                    /* conversion results: */
                    /* adc_data[0] -> should have conversions for Group1 channel 6 */

                    char text[38] = {0};
                    snprintf(text, 33, "Ambient Light ADC Value: %d\r\n", (int)adc_data[0].value);
                    sciPrintText(scilinREG, (unsigned char *)text, 38);
                    break;
            }
        }
    }
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    return xQueueSend(lightServiceQueueHandle, (void *) event, portMAX_DELAY);
}