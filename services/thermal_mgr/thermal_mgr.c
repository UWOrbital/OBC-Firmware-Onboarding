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
  /* Send an event to the thermal manager queue */
  event->type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD;
  if (xQueueSend(thermalMgrQueueHandle, event, 0) == pdPASS) {
     return ERR_CODE_SUCCESS;
  } else {
    return ERR_CODE_QUEUE_FULL; 
  }
}

void osHandlerLM75BD(void) { //function only runs when an interrupt happens
  /* Implement this function */
  float tempC = 0.0f;
  if (readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempC) == ERR_CODE_SUCCESS) {
    if (tempC >= 75.0f) { //if an interrupt happens and the temp is above 75you
      overTemperatureDetected();
    }  else {
      safeOperatingConditions();
    }
  }
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  lm75bd_config_t *config = (lm75bd_config_t *)pvParameters;
  thermal_mgr_event_t event;
  float temperatureC = 0.0f;
  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdTRUE) {
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
        if (readTempLM75BD(config->devAddr, &temperatureC) == ERR_CODE_SUCCESS) {
          addTemperatureTelemetry(temperatureC);
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
