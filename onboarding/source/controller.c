#include "controller.h"
#include "amb_light_service.h"
#include "serial_io.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_timer.h>
#include <sys_common.h>

#include <sci.h>

#include <string.h>

TaskHandle_t controllerTaskHandle = NULL;
TimerHandle_t lightTimerHandle = NULL;

void controllerTask(void * pvParameters);
static void lightTimerCallback(TimerHandle_t xTimer);

uint8_t initController(void) {
    if ( (controllerTaskHandle != NULL) && (lightTimerHandle != NULL) ) {
        return 1;
    }
    BaseType_t xReturned;
    xReturned = xTaskCreate(controllerTask,             /* Function that implements the task. */
                            CONTROLLER_NAME,            /* Text name for the task. */
                            CONTROLLER_STACK_SIZE,      /* Stack size in words, not bytes. */
                            NULL,                       /* Parameter passed into the task. */
                            CONTROLLER_PRIORITY,        /* Priority at which the task is created. */
                            &controllerTaskHandle);     /* Used to pass out the created task's handle. */
    
    if (xReturned != pdPASS) {
        return 0;
    }

    lightTimerHandle = xTimerCreate(LIGHT_TIMER_NAME,        /* Just a text name, not used by the RTOS kernel. */
                                LIGHT_TIMER_PERIOD,      /* The timer period in ticks, must be greater than 0. */
                                LIGHT_TIMER_AUTORELOAD,  /* The timers will auto-reload themselves when they expire. */
                                (void *) 0,              /* The ID is used to store a count of the number of times the timer has expired, which is initialised to 0. */
                                lightTimerCallback);     /* Each timer calls the same callback when it expires. */

    if (lightTimerHandle == NULL) {
        return 0;
    }
    return 1;
}

void controllerTask(void * pvParameters) {
    ASSERT(controllerTaskHandle != NULL);
    ASSERT(lightTimerHandle != NULL);

    if (initLightService() == 0) {
        /* Failed to initialize light service */
        sciPrintText(scilinREG, (unsigned char *)"Failed to initialize light service\r\n", strlen("Failed to initialize light service\r\n"));
    } else {        
        if (xTimerStart(lightTimerHandle, 0) != pdPASS) {
            /* Timer could not be started */
            sciPrintText(scilinREG, (unsigned char *)"Failed to start light service timer\r\n", strlen("Failed to start light service timer\r\n"));
        }
    }

    for (;;) {}
}

static void lightTimerCallback(TimerHandle_t xTimer) {
    ASSERT(xTimer != NULL);

    light_event_t event = MEASURE_LIGHT;
    if (sendToLightServiceQueue(&event) == 0) {
        /* Failed to send event to light service */
        sciPrintText(scilinREG, (unsigned char *)"Failed to send event to light service\r\n", strlen("Failed to send event to light service\r\n"));
    }
}
