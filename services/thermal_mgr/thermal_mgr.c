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
  
  // Technically, we should use the fromISR variant of this for osHandler
  if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) {
    return ERR_CODE_QUEUE_FULL;
  }
  
  return ERR_CODE_SUCCESS;
}

error_code_t eventHandler(lm75bd_config_t *config) {
  error_code_t errCode = ERR_CODE_SUCCESS;

  if (config == NULL) return ERR_CODE_INVALID_ARG;

  thermal_mgr_event_t event = {0};
  xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY);

  switch (event.type) {
    case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD: {
      float tempC;
      errCode = readTempLM75BD(config->devAddr, &tempC);
      if (errCode != ERR_CODE_SUCCESS) break;

      addTemperatureTelemetry(tempC);
      break;
    }
    case THERMAL_MGR_EVENT_OS_INT_DETECTED: {
      float tempC;
      errCode = readTempLM75BD(config->devAddr, &tempC);
      if (errCode != ERR_CODE_SUCCESS) break;

      printConsole("Getting context for OS interrupt -> Temperature: %f\n", tempC);
      
      if (tempC >= config->overTempThresholdCelsius) {
        overTemperatureDetected();
      } else if (tempC <= config->hysteresisThresholdCelsius) {
        safeOperatingConditions();
      }

      break;
    }
    default:
      errCode = ERR_CODE_INVALID_QUEUE_MSG;
      break;
  }

  return errCode;
}

void addTemperatureTelemetry(float tempC) {
  printConsole("Temperature telemetry: %f deg C\n", tempC);
}

static void thermalMgr(void *pvParameters) {
  lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;
    
  while (1) {
    eventHandler(&config);
  }
}

void overTemperatureDetected(void) {
  printConsole("Over temperature detected!\n");
}

void safeOperatingConditions(void) { 
  printConsole("Returned to safe operating conditions!\n");
}

error_code_t osHandlerLM75BD(uint8_t devAddr) {
  /* Implement this driver function */
  thermal_mgr_event_t event = {.type = THERMAL_MGR_EVENT_OS_INT_DETECTED};
  thermalMgrSendEvent(&event);
  return ERR_CODE_SUCCESS;
}
