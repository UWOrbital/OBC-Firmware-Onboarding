#include "controller.h"
#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"

#include <FreeRTOS.h>
#include <os_projdefs.h>
#include <os_task.h>
#include <os_timer.h>
#include <sys_common.h>

#include <gio.h>
#include <sci.h>

#include <string.h>

/* Controller task config */
#define CONTROLLER_NAME         "controller"
#define CONTROLLER_STACK_SIZE   256
#define CONTROLLER_PRIORITY     1

/* LED timer config */
#define LED_TIMER_NAME          "led_timer"
#define LED_TIMER_PERIOD        pdMS_TO_TICKS(2500)
#define LED_TIMER_AUTORELOAD    pdTRUE

/* USER CODE BEGIN */
// define config for the light timer

/* USER CODE END */

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

obc_error_code_t initController(void) {
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

    if (xReturned != pdPASS)
        return OBC_ERROR_TASK_CREATION_FAILED;

    if (ledTimerHandle == NULL) {
        // Create led timer
        ledTimerHandle = xTimerCreate(LED_TIMER_NAME,
                                    LED_TIMER_PERIOD,
                                    LED_TIMER_AUTORELOAD,
                                    (void *) 0,
                                    ledTimerCallback);
    }

    if (ledTimerHandle == NULL)
        return OBC_ERROR_TIMER_CREATION_FAILED;

    /* USER CODE BEGIN */
    // Create light timer and check if it was created successfully

    /* USER CODE END */
}

static void controllerTask(void * pvParameters) {
    ASSERT(controllerTaskHandle != NULL);
    ASSERT(ledTimerHandle != NULL);

    obc_error_code_t lightServiceStatus = initLightService();
    if (lightServiceStatus != OBC_ERR_CODE_SUCCESS) {
        /* USER CODE BEGIN */
        // Deal with error when initializing light service task and/or queue

        /* USER CODE END */
    } else { 
        /* Light service task and queue created successfully */
        BaseType_t xReturned; 
        xReturned = xTimerStart(ledTimerHandle, 0);
        
        /* USER CODE BEGIN */
        // Start light timer and check if both timers were started successfully

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

    /* USER CODE END */
}

