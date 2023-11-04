#include "thermal_mgr.h"
#include "console.h"
#include "errors.h"
#include "lm75bd.h"
#include "logging.h"
#include "os_portmacro.h"
#include "os_projdefs.h"

#include <FreeRTOS.h>
#include <os_queue.h>
#include <os_task.h>

#include <stdint.h>
#include <string.h>

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

  thermalMgrTaskHandle =
      xTaskCreateStatic(thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE,
                        config, 1, thermalMgrTaskStack, &thermalMgrTaskBuffer);

  memset(&thermalMgrQueueBuffer, 0, sizeof(thermalMgrQueueBuffer));
  memset(thermalMgrQueueStorageArea, 0, sizeof(thermalMgrQueueStorageArea));

  thermalMgrQueueHandle =
      xQueueCreateStatic(THERMAL_MGR_QUEUE_LENGTH, THERMAL_MGR_QUEUE_ITEM_SIZE,
                         thermalMgrQueueStorageArea, &thermalMgrQueueBuffer);
}

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  /* Send an event to the thermal manager queue */
  if (!event)
    return ERR_CODE_NULL_ARG;
  if (xQueueSend(thermalMgrQueueHandle, event, (TickType_t)0) == pdTRUE)
    return ERR_CODE_SUCCESS;
  else
    return ERR_CODE_QUEUE_FULL;
}

void osHandlerLM75BD(void) { /* Implement this function */
  thermal_mgr_event_t event;
  event.type = THERNAL_MGR_EVENT_CHECK_OS;
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {
  lm75bd_config_t details = *(lm75bd_config_t *)pvParameters;
  while (1) {
    thermal_mgr_event_t event;
    if (xQueueReceive(thermalMgrQueueHandle, &event, (TickType_t)10) ==
        pdTRUE) {
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD ||
          event.type == THERNAL_MGR_EVENT_CHECK_OS) {
        float val;
        error_code_t errCode = readTempLM75BD(details.devAddr, &val);
        if (errCode != ERR_CODE_SUCCESS) {
          LOG_ERROR_CODE(errCode);
          // ensure that if CHECK_OS event was sent, that the temperature gets
          // if it failed the first time attempt to CHECK_OS again
          if (event.type == THERNAL_MGR_EVENT_CHECK_OS)
            thermalMgrSendEvent(&event);
          continue;
        }
        if (event.type == THERNAL_MGR_EVENT_CHECK_OS) {
          if (val > details.hysteresisThresholdCelsius)
            overTemperatureDetected();
          else
            safeOperatingConditions();
        }
        addTemperatureTelemetry(val);
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
