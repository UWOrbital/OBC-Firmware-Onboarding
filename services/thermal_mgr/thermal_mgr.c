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
  if (event == NULL) {
    return ERR_CODE_INVALID_ARG;
  }
  if (xQueueSend(thermalMgrQueueHandle, event, 0) == errQUEUE_FULL) {
    return ERR_CODE_QUEUE_FULL;
  }
  return ERR_CODE_SUCCESS;
}

// this has to be non blocking i think so im not running the reading thing within this?
// not sure if this actually makes sense
void osHandlerLM75BD(void) {
  thermal_mgr_event_t thermalMgrEvent = {THERMAL_MGR_EVENT_OS};
  thermalMgrSendEvent(&thermalMgrEvent);
}

static void thermalMgr(void *pvParameters) {
  lm75bd_config_t data = *(lm75bd_config_t *) pvParameters;
  thermal_mgr_event_t event;
  float temp = 0;

  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY)) {
      error_code_t errCode = readTempLM75BD(data.devAddr, &temp);
      if (errCode != ERR_CODE_SUCCESS) {
        LOG_ERROR("LM75BD temperature read failed! Code: %lu", (uint32_t)errCode); // ! is this adequate error logging
        continue; // skip logic if driver function fails
      }
      switch (event.type) {
        case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
          addTemperatureTelemetry(temp);
        case THERMAL_MGR_EVENT_OS:
          if (temp > data.hysteresisThresholdCelsius) {
            overTemperatureDetected();
          }
          else {
            safeOperatingConditions();
          }
        default:
          // ! what behavior is expected if its not one of those events?
          // i guess a warn makes sense
          LOG_WARN("Unexpected thermal manager event recieved! Code: %lu", (uint32_t)event.type);
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
