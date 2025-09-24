#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"
#include "logging.h"

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
  BaseType_t result;
  if(event != NULL && thermalMgrQueueHandle != NULL){
    result = xQueueSend(thermalMgrQueueHandle, event, 0);
  } else {
    return ERR_CODE_INVALID_ARG;
  }

  if(result == pdPASS){
    return ERR_CODE_SUCCESS;
  } else {
    return ERR_CODE_QUEUE_FULL;
  }
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t event = { .type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD };
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  thermal_mgr_event_t event;
  lm75bd_config_t *config = pvParameters;

  float temp;
  while (1) {
    BaseType_t result = xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY);
    
    if(result == pdPASS && event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
      readTempLM75BD(config->devAddr, &temp);
      if(temp > config->overTempThresholdCelsius){ //if temperature is greater than 80
        overTemperatureDetected();
      } else if (temp < config->hysteresisThresholdCelsius) { //if temperature is less than 75
        safeOperatingConditions(); 
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
