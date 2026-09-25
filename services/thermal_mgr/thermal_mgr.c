#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include <string.h>

#define THERMAL_MGR_STACK_SIZE 256U
#define OVERTEMPERATURE 80
#define HYSTERESIS_THRESHOLD 75

static TaskHandle_t thermalMgrTaskHandle;
static StaticTask_t thermalMgrTaskBuffer;
static StackType_t thermalMgrTaskStack[THERMAL_MGR_STACK_SIZE];

#define THERMAL_MGR_QUEUE_LENGTH 10U
#define THERMAL_MGR_QUEUE_ITEM_SIZE sizeof(thermal_mgr_event_t)

static QueueHandle_t thermalMgrQueueHandle;
static StaticQueue_t thermalMgrQueueBuffer;
static uint8_t thermalMgrQueueStorageArea[THERMAL_MGR_QUEUE_LENGTH * THERMAL_MGR_QUEUE_ITEM_SIZE];

static void thermalMgr(void *pvParameters);

void initThermalSystemManager(lm75bd_config_t *config)
{
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

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event)
{
    if (event == NULL)
        return ERR_CODE_INVALID_ARG;

    if (xQueueSend(thermalMgrQueueHandle, event, (TickType_t)10))
        return ERR_CODE_SUCCESS;
    return ERR_CODE_QUEUE_FULL;
}

void osHandlerLM75BD(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    thermal_mgr_event_t event = {.type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD};

    // Get the thermal manager to deal with the temperature reading
    xQueueSendFromISR(thermalMgrQueueHandle, &event, &xHigherPriorityTaskWoken);
}

static void thermalMgr(void *pvParameters)
{
    lm75bd_config_t configData = *(lm75bd_config_t *)pvParameters;
    int overTemperatureState = 0;

    while (1)
    {
        void *pvBuffer;
        xQueueReceive(thermalMgrQueueHandle, pvBuffer, (TickType_t)10);

        thermal_mgr_event_t eventType = *(thermal_mgr_event_t *)pvBuffer;
        if (eventType.type != THERMAL_MGR_EVENT_MEASURE_TEMP_CMD)
            continue;

        float temp = 0.0;
        if (readTempLM75BD(configData.devAddr, &temp) != ERR_CODE_SUCCESS)
            continue;

        addTemperatureTelemetry(temp);

        if (temp > OVERTEMPERATURE && overTemperatureState == 0)
        {
            overTemperatureState = 1;
            overTemperatureDetected();
        }
        else if (temp < HYSTERESIS_THRESHOLD && overTemperatureState == 1)
        {
            overTemperatureState = 0;
            safeOperatingConditions();
        }
    }
}

void addTemperatureTelemetry(float tempC)
{
    printConsole("Temperature telemetry: %f deg C\n", tempC);
}

void overTemperatureDetected(void)
{
    printConsole("Over temperature detected!\n");
}

void safeOperatingConditions(void)
{
    printConsole("Returned to safe operating conditions!\n");
}
