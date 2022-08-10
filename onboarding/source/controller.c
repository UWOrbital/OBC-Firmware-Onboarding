#include "controller.h"
#include "amb_light_service.h"
#include "serial_io.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_timer.h>
#include <sys_common.h>

#include <gio.h>
#include <sci.h>

#include <string.h>

/* Declare handlers for tasks and timers */
static TaskHandle_t controllerTaskHandle = NULL;
static TimerHandle_t lightTimerHandle = NULL;
static TimerHandle_t ledTimerHandle = NULL;

/**
 * @brief Task that starts the led and light timers.
 * @param pvParameters Task parameters
 */
static void controllerTask(void * pvParameters);

/**
 * @brief The light timer callback function that sends a MEASURE_LIGHT event to the light service queue.
 */
static void lightTimerCallback(TimerHandle_t xTimer);

/**
 * @brief The led timer callback function that toggles the LED.
 */
static void ledTimerCallback(TimerHandle_t xTimer);

uint8_t initController(void) {
    BaseType_t xReturned = pdFAIL;
    if (controllerTaskHandle == NULL) {
        // Create controller task
        xReturned = xTaskCreate(controllerTask,             /* Function that implements the task. */
                                CONTROLLER_NAME,            /* Text name for the task. */
                                CONTROLLER_STACK_SIZE,      /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                CONTROLLER_PRIORITY,        /* Priority at which the task is created. */
                                &controllerTaskHandle);     /* Used to pass out the created task's handle. */
    }

    if (ledTimerHandle == NULL) {
        // Create led timer
        ledTimerHandle = xTimerCreate(LED_TIMER_NAME,
                                    LED_TIMER_PERIOD,
                                    LED_TIMER_AUTORELOAD,
                                    (void *) 0,
                                    ledTimerCallback);
    }

    /* USER CODE BEGIN */
    // Create light timer and check if task/timers were created successfully
    if (lightTimerHandle == NULL) {
        lightTimerHandle = xTimerCreate(LIGHT_TIMER_NAME,        /* Just a text name, not used by the RTOS kernel. */
                                    LIGHT_TIMER_PERIOD,      /* The timer period in ticks, must be greater than 0. */
                                    LIGHT_TIMER_AUTORELOAD,  /* The timers will auto-reload themselves when they expire. */
                                    (void *) 0,              /* The ID is used to store a count of the number of times the timer has expired, which is initialised to 0. */
                                    lightTimerCallback);     /* Each timer calls the same callback when it expires. */    
    }
    
    if ( (xReturned != pdPASS) || (lightTimerHandle == NULL) || (ledTimerHandle == NULL) ) {
        return 0;
    }
    /* USER CODE END */

    return 1;
}

static void controllerTask(void * pvParameters) {
    ASSERT(controllerTaskHandle != NULL);
    ASSERT(ledTimerHandle != NULL);

    uint8_t lightServiceStatus = initLightService();
    if (lightServiceStatus == 0) {
        /* USER CODE BEGIN */
        // Deal with error when initializing light service task and/or queue
        sciPrintText(scilinREG, (unsigned char *)"Failed to initialize light service\r\n", strlen("Failed to initialize light service\r\n"));
        /* USER CODE END */
    } else { 
        /* Light service task and queue created successfully */
        BaseType_t xReturned = xTimerStart(ledTimerHandle, 0);
        
        /* USER CODE BEGIN */
        // Start light timer
        BaseType_t xReturnedLightTimer = xTimerStart(lightTimerHandle, 0);

        // Check if timers were started successfully
        if ( (xReturned != pdPASS) || (xReturnedLightTimer != pdPASS) ) {
            // Deal with error when starting timers
            sciPrintText(scilinREG, (unsigned char *)"Failed to start at least 1 of the timers\r\n", strlen("Failed to start at least one of the timers\r\n"));
        }
        /* USER CODE END */
    }

    while (1);
}

static void ledTimerCallback(TimerHandle_t xTimer) {
    ASSERT(xTimer != NULL);

    gioToggleBit(gioPORTB, 1);
}

static void lightTimerCallback(TimerHandle_t xTimer) {
    /* USER CODE BEGIN */
    // Send light event to light service queue
    ASSERT(xTimer != NULL);

    light_event_t event = MEASURE_LIGHT;
    if (sendToLightServiceQueue(&event) == 0) {
        /* Failed to send event to light service */
        sciPrintText(scilinREG, (unsigned char *)"Failed to send event to light service\r\n", strlen("Failed to send event to light service\r\n"));
    }
    /* USER CODE END */
}

