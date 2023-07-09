#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <string.h>

#define THERMAL_MGR_STACK_SIZE 256U

static TaskHandle_t thermalMgrTaskHandle;
static StaticTask_t thermalMgrTaskBuffer;
static StackType_t thermalMgrTaskStack[THERMAL_MGR_STACK_SIZE];

static void thermalMgr(void *pvParameters);

void initThermalSystemManager(lm75bd_config_t *config) {
    memset(&thermalMgrTaskBuffer, 0, sizeof(thermalMgrTaskBuffer));
    memset(thermalMgrTaskStack, 0, sizeof(thermalMgrTaskStack));
    
    thermalMgrTaskHandle = xTaskCreateStatic(
        thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE,
        config, 1, thermalMgrTaskStack, &thermalMgrTaskBuffer);
}

static void thermalMgr(void *pvParameters) {
    const lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;
    
    while (1) {
        float tempC = 0.0f;
        printConsole("Measured temperature: %f\n", tempC);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
