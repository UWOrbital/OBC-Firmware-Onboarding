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
  xQueueSend(thermalMgrQueueHandle, event, (TickType_t) 10);

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t event;
  thermal_mgr_event_type_t overtempInterrupt = THERMAL_MGR_EVENT_OVER_TEMP_INTERRUPT;
  event.type = overtempInterrupt;
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  lm75bd_config_t data = *(lm75bd_config_t *) pvParameters;
  float temp = 0;

  while (1) {
    thermal_mgr_event_t recvEvent;
    if(thermalMgrQueueHandle != NULL && xQueueReceive(thermalMgrQueueHandle, &recvEvent, (TickType_t) 10) == pdPASS) {
      thermal_mgr_event_type_t measureTempCmd = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD;
      thermal_mgr_event_type_t overtempInterrupt = THERMAL_MGR_EVENT_OVER_TEMP_INTERRUPT;
      if(recvEvent.type == measureTempCmd) {
        readTempLM75BD(data.devAddr, &temp);
        addTemperatureTelemetry(temp);
      } else if(recvEvent.type == overtempInterrupt) {
        readTempLM75BD(data.devAddr, &temp);
        if(temp > data.hysteresisThresholdCelsius) {
          overTemperatureDetected();
        } else {
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
