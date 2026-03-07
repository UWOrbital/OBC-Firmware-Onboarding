#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"
#include "logging.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>
#include <stdbool.h>

#include <string.h>

#define THERMAL_MGR_STACK_SIZE 256U

static TaskHandle_t thermalMgrTaskHandle;
static StaticTask_t thermalMgrTaskBuffer;
static StackType_t thermalMgrTaskStack[THERMAL_MGR_STACK_SIZE];

#define THERMAL_MGR_QUEUE_LENGTH 10U
#define THERMAL_MGR_QUEUE_ITEM_SIZE sizeof(thermal_mgr_event_t)

static QueueHandle_t thermalMgrQueueHandle;
static StaticQueue_t thermalMgrQueueBuffer;
static uint8_t thermalMgrQueueStorageArea[THERMAL_MGR_QUEUE_LENGTH * THERMAL_MGR_QUEUE_ITEM_SIZE];

static void thermalMgr(void *pvParameters);

void initThermalSystemManager(lm75bd_config_t *config) {
    memset(&thermalMgrTaskBuffer, 0, sizeof(thermalMgrTaskBuffer));
    memset(thermalMgrTaskStack, 0, sizeof(thermalMgrTaskStack));

    thermalMgrTaskHandle = xTaskCreateStatic(
        thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE,
        config, 1, thermalMgrTaskStack, &thermalMgrTaskBuffer);

    memset(&thermalMgrQueueBuffer, 0, sizeof(thermalMgrQueueBuffer));
    memset(thermalMgrQueueStorageArea, 0, sizeof(thermalMgrQueueStorageArea));

    thermalMgrQueueHandle = xQueueCreateStatic(
        THERMAL_MGR_QUEUE_LENGTH, THERMAL_MGR_QUEUE_ITEM_SIZE,
        thermalMgrQueueStorageArea, &thermalMgrQueueBuffer);

}

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  /* Send an event to the thermal manager queue */
    if (!event) {
        return ERR_CODE_INVALID_ARG;
    }

    if (!thermalMgrQueueHandle) {
        return ERR_CODE_INVALID_QUEUE_MSG;
    }

    if (xQueueSend(thermalMgrQueueHandle, event, 10) != pdPASS) {
        return ERR_CODE_QUEUE_FULL;
    }

    return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
    /* Implement this function */
    const uint8_t addr = LM75BD_OBC_I2C_ADDR;
    thermal_mgr_event_t e = {.type = THERMAL_MGR_EVENT_DETECT_TEMP_SAFETY_CMD};
    error_code_t errCode;
    LOG_IF_ERROR_CODE(thermalMgrSendEvent(&e));
}

static void thermalMgr(void *pvParameters) {
    /* Implement this task */
    const lm75bd_config_t config = *(lm75bd_config_t*)pvParameters;

#ifndef PORTMACRO_H
#error BaseType_t not defined
#endif

    while (1) {
        thermal_mgr_event_t event;
        const BaseType_t xReturned = xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY);

        if (xReturned != pdPASS) {
            LOG_ERROR_CODE(ERR_CODE_INVALID_QUEUE_MSG);
            continue;   // fetch the next message
        }

        switch(event.type) {
            case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD: {
                float t;
                error_code_t errCode;
                LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &t));

                if (errCode == ERR_CODE_SUCCESS) {
                    addTemperatureTelemetry(t);
                }
                break;
            }
            case THERMAL_MGR_EVENT_DETECT_TEMP_SAFETY_CMD: {
                float t;
                error_code_t errCode;
                LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &t));

                if (errCode == ERR_CODE_SUCCESS) {
                    if (t >= config.overTempThresholdCelsius) {
                        overTemperatureDetected();
                    }
                    else if (t <= config.hysteresisThresholdCelsius) {
                        safeOperatingConditions();
                    }
                }
                break;
            }
            default:
                break;
        };
    }
}

void addTemperatureTelemetry(float tempC) {
    printConsole("Temperature telemetry: %f deg C\n", tempC);
}

void overTemperatureDetected(void) {
    printConsole("Over temperature detected!\n");
}

void safeOperatingConditions(void) {
    printConsole("Returned to safe operating conditions!\n");
}
