#include "thermal_mgr.h"

#include <FreeRTOS.h>
#include <os_queue.h>
#include <os_task.h>
#include <string.h>

#include "console.h"
#include "errors.h"
#include "lm75bd.h"

#define THERMAL_MGR_STACK_SIZE 256U

static TaskHandle_t thermalMgrTaskHandle;
static StaticTask_t thermalMgrTaskBuffer;
static StackType_t thermalMgrTaskStack[THERMAL_MGR_STACK_SIZE];

#define THERMAL_MGR_QUEUE_LENGTH 10U
#define THERMAL_MGR_QUEUE_ITEM_SIZE sizeof(thermal_mgr_event_t)

static QueueHandle_t thermalMgrQueueHandle;
static StaticQueue_t thermalMgrQueueBuffer;
static uint8_t thermalMgrQueueStorageArea[THERMAL_MGR_QUEUE_LENGTH *
                                          THERMAL_MGR_QUEUE_ITEM_SIZE];

static void thermalMgr(void *pvParameters);

void initThermalSystemManager(lm75bd_config_t *config) {
    memset(&thermalMgrTaskBuffer, 0, sizeof(thermalMgrTaskBuffer));
    memset(thermalMgrTaskStack, 0, sizeof(thermalMgrTaskStack));

    thermalMgrTaskHandle = xTaskCreateStatic(
        thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE, config, 1,
        thermalMgrTaskStack, &thermalMgrTaskBuffer);

    memset(&thermalMgrQueueBuffer, 0, sizeof(thermalMgrQueueBuffer));
    memset(thermalMgrQueueStorageArea, 0, sizeof(thermalMgrQueueStorageArea));

    thermalMgrQueueHandle = xQueueCreateStatic(
        THERMAL_MGR_QUEUE_LENGTH, THERMAL_MGR_QUEUE_ITEM_SIZE,
        thermalMgrQueueStorageArea, &thermalMgrQueueBuffer);
}

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
    if (event == NULL) return ERR_CODE_INVALID_ARG;

    if (xQueueSend(thermalMgrQueueHandle, (void *)event, (TickType_t)0) ==
        errQUEUE_FULL) {
        return ERR_CODE_QUEUE_FULL;
    }

    return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
    thermal_mgr_event_t interruptEvent;
    interruptEvent.type = THERMAL_MGR_EVENT_INTERRUPT_CMD;
    thermalMgrSendEvent(&interruptEvent);
}

static void thermalMgr(void *pvParameters) {
    if (pvParameters == NULL) return;

    lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;
    thermal_mgr_event_t taskEvent;
    error_code_t taskStatus;
    float temp = 0;

    while (1) {
        if (xQueueReceive(thermalMgrQueueHandle, &taskEvent, portMAX_DELAY) ==
            pdTRUE) {
            if (taskEvent.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
                taskStatus = readTempLM75BD(config.devAddr, &temp);
                if (taskStatus != ERR_CODE_SUCCESS) continue;
                addTemperatureTelemetry(temp);
            } else if (taskEvent.type == THERMAL_MGR_EVENT_INTERRUPT_CMD) {
                taskStatus = readTempLM75BD(config.devAddr, &temp);
                if (taskStatus != ERR_CODE_SUCCESS) continue;
                if (temp > config.overTempThresholdCelsius) {
                    overTemperatureDetected();
                } else if (temp < config.hysteresisThresholdCelsius) {
                    safeOperatingConditions();
                }
            }
        }
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
