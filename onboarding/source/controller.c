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

    // Check if task was created successfully
    if (xReturned == pdFAIL) {
        unsigned char* errorMsg = (unsigned char*) "Error creating controller task.\n";
        sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
        return 0;
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
        // Create light timer
        lightTimerHandle = xTimerCreate(LIGHT_TIMER_NAME,
                                        LIGHT_TIMER_PERIOD,
                                        LIGHT_TIMER_AUTORELOAD,
                                        (void *) 0,
                                        lightTimerCallback);
    }

    // Check if timers were created successfully
    if (ledTimerHandle == NULL) {
        unsigned char* errorMsg = (unsigned char*) "Error creating led timer.\n";
        sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
    }
    if (lightTimerHandle == NULL) {
        unsigned char* errorMsg = (unsigned char*) "Error creating light timer.\n";
        sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
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
        unsigned char* errorMsg = (unsigned char*) "Error initializing light service task and/or queue.\n";
        sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
        /* USER CODE END */
    } else { 
        /* Light service task and queue created successfully */
        BaseType_t xReturned = xTimerStart(ledTimerHandle, 0);
        
        // Check if led timer was started successfully
        if (xReturned == pdFAIL) {
            unsigned char* errorMsg = (unsigned char*) "Error starting led timer.\n";
            sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
        }
        
        /* USER CODE BEGIN */
        // Start light timer
        xReturned = xTimerStart(lightTimerHandle, 0);

        // Check if led timer was started successfully
        if (xReturned == pdFAIL) {
            unsigned char* errorMsg = (unsigned char*) "Error starting light timer.\n";
            sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
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

    light_event_t event = MEASURE_LIGHT;
    uint8_t returned = sendToLightServiceQueue(&event);
    if (returned == 0) {
        unsigned char* errorMsg = (unsigned char*) "Error sending to light service queue.\n";
        sciPrintText(scilinREG, errorMsg, strlen((const char*) errorMsg));
    }
    /* USER CODE END */
}

