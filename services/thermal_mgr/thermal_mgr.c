#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

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
    if (thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;
    if (event == NULL) return ERR_CODE_INVALID_ARG;
    
    if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) {
        return ERR_CODE_QUEUE_FULL;
    }
    
    return ERR_CODE_SUCCESS;
}

static void thermalMgr(void *pvParameters) {
    const lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;
    
    while (1) {
        thermal_mgr_event_t event;
        xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY);

        switch (event.type) {
            case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD: {
                float tempC = 0.0f;
                printConsole("Measured temperature: %f\n", tempC);
                break;
            }
            case THERMAL_MGR_EVENT_OVERTEMP_DETECTED:
                // Check if we've entered or exited overtemperature state
                printConsole("Overtemperature detected!\n");
                break;
            default:
                printConsole("Invalid event type received!\n");
                break;
        }
    }
}
