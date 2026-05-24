#include "thermal_mgr.h"

#include <FreeRTOS.h>
#include <os_queue.h>
#include <os_task.h>
#include <string.h>

#include "console.h"
#include "errors.h"
#include "lm75bd.h"
#include "logging.h"

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

static void thermalMgr(void* pvParameters);

void initThermalSystemManager(lm75bd_config_t* config) {
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

error_code_t thermalMgrSendEvent(thermal_mgr_event_t* event) {
  if (event == NULL) {
    return ERR_CODE_INVALID_ARG;
  }

  /* Send an event to the thermal manager queue */
  if (thermalMgrQueueHandle != NULL) {
    if (xQueueSend(thermalMgrQueueHandle, event, (TickType_t)10) == pdTRUE) {
      return ERR_CODE_SUCCESS;
    } else {
      return ERR_CODE_QUEUE_FULL;
    }
  } else {
    return ERR_CODE_INVALID_STATE;
  }
}

void osHandlerLM75BD(void) {
  thermal_mgr_event_type_t event = THERMAL_MGR_EVENT_OVERHEAT; 
  // this should be fine since xQueue makes a copy, event is on the stack long enough for this to happen
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void* pvParameters) {
  float temp;
  thermal_mgr_event_t event;

  lm75bd_config_t config = *(lm75bd_config_t *) pvParameters;

  while (1) {
    if (thermalMgrQueueHandle != NULL &&
        xQueueReceive(thermalMgrQueueHandle, &event,
                      (TickType_t)portMAX_DELAY) == pdTRUE) {
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
        error_code_t errCode;
        LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temp));
        if (errCode == ERR_CODE_SUCCESS) {
          addTemperatureTelemetry(temp);
        }
      } else if (event.type == THERMAL_MGR_EVENT_OVERHEAT) {
        error_code_t errCode;
         LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temp));
        if (errCode == ERR_CODE_SUCCESS) {
          if (temp > config.hysteresisThresholdCelsius) {
            overTemperatureDetected();
          } else {
            safeOperatingConditions();
          }
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
