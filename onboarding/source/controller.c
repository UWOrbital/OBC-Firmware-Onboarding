#include "controller.h"
#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"

#include <FreeRTOS.h>
#include <os_projdefs.h>
#include <os_task.h>
#include <os_timer.h>
#include <sys_common.h>
#include <stdio.h>

#include <gio.h>
#include <sci.h>

#include <string.h>

/* Controller task config */
#define CONTROLLER_NAME         "controller"
#define CONTROLLER_STACK_SIZE   256UL
#define CONTROLLER_PRIORITY     1UL

/* LED timer config */
#define LED_TIMER_NAME          "led_timer"
#define LED_TIMER_PERIOD        pdMS_TO_TICKS(2500)
#define LED_TIMER_AUTORELOAD    pdTRUE

/* USER CODE BEGIN */
// define config for the light timer

/* USER CODE END */

/* Declare handlers and buffers for tasks and timers */
static TaskHandle_t controllerTaskHandle;
static StaticTask_t controllerTaskBuffer;
static StackType_t controllerTaskStack[CONTROLLER_STACK_SIZE];

static TimerHandle_t lightTimerHandle;
static StaticTimer_t lightTimerBuffer;

static TimerHandle_t ledTimerHandle;
static StaticTimer_t ledTimerBuffer;

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
    if (controllerTaskHandle == NULL) {
        // Create controller task
<<<<<<< HEAD
        xReturned = xTaskCreate(controllerTask,             /* Function that implements the task. */
                                CONTROLLER_NAME,            /* Text name for the task. */
                                CONTROLLER_STACK_SIZE,      /* Stack size in words, not bytes. */
                                NULL,                       /* Parameter passed into the task. */
                                CONTROLLER_PRIORITY,        /* Priority at which the task is created. */
                                &controllerTaskHandle);     /* Used to pass out the created task's handle. */
        if (xReturned == pdFAIL) {
                printf("\ncontrollerTask not created...\n");
            }
        }
=======
        controllerTaskHandle = xTaskCreateStatic(   controllerTask,             /* Function that implements the task. */
                                                    CONTROLLER_NAME,            /* Text name for the task. */
                                                    CONTROLLER_STACK_SIZE,      /* Stack size in words, not bytes. */
                                                    NULL,                       /* Parameter passed into the task. */
                                                    CONTROLLER_PRIORITY,        /* Priority at which the task is created. */
                                                    controllerTaskStack,        /* Array to use as the task's stack. */
                                                    &controllerTaskBuffer);     /* Used to hold the task's data structure */
    }
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d

    if (controllerTaskHandle == NULL)
        return OBC_ERR_CODE_TASK_CREATION_FAILED;

    if (ledTimerHandle == NULL) {
        // Create led timer
        ledTimerHandle = xTimerCreateStatic(LED_TIMER_NAME,
                                            LED_TIMER_PERIOD,
                                            LED_TIMER_AUTORELOAD,
                                            (void *) 0,
                                            ledTimerCallback,
                                            &ledTimerBuffer);
    }

    if (ledTimerHandle == NULL)
        return OBC_ERR_CODE_TIMER_CREATION_FAILED;

    /* USER CODE BEGIN */
<<<<<<< HEAD
   if (lightTimerHandle == NULL) {
    // Create light timer
       lightTimerHandle = xTimerCreate(LIGHT_CONTROLLER_NAME,
                                    LIGHT_CONTROLLER_PERIOD, 
                                    LIGHT_CONTROLLER_AUTORELOAD,
                                    (void *) 0,
                                    lightTimerCallback);
                                    
         if(lightTimerHandle == NULL || xReturned == pdFAIL) {
            sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
        }
    }

    /* USER CODE END */

        return 1;
=======
    // Create light timer and check if it was created successfully

    /* USER CODE END */
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d
}

static void controllerTask(void * pvParameters) {
    ASSERT(controllerTaskHandle != NULL);
    ASSERT(ledTimerHandle != NULL);

    obc_error_code_t lightServiceStatus = initLightService();
    if (lightServiceStatus != OBC_ERR_CODE_SUCCESS) {
        /* USER CODE BEGIN */
        sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));

    } else { 
        /* USER CODE END 
 
        /* Light service task and queue created successfully */
        BaseType_t xReturned; 
        xReturned = xTimerStart(ledTimerHandle, 0);
        
        /* USER CODE BEGIN */
<<<<<<< HEAD
        xReturned = xTimerStart(lightTimerHandle, 0);

        if (xReturned == pdFAIL) {
            sciPrintText(scilinREG, (unsigned char *) ERROR_MESSAGE, sizeof(ERROR_MESSAGE));
        }
            
=======
        // Start light timer and check if both timers were started successfully
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d

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
    light_event_t xlightevent = MEASURE_LIGHT;
    sendToLightServiceQueue(&xlightevent);

    /* USER CODE END */
}

