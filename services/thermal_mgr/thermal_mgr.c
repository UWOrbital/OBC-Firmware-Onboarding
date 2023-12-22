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
  // If no event is given arguments are bad
  if (event == NULL) return ERR_CODE_INVALID_ARG;
  // If queue handle doesnt describe a queue, error in state
  if (thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;
  // If queue is full, cannot add event, error
  if (uxQueueSpacesAvailable(thermalMgrQueueHandle) == 0) return ERR_CODE_QUEUE_FULL;
  xQueueSend(thermalMgrQueueHandle, event, 10);
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  error_code_t errCode;
  float tempReading = 0.0;
  //Imported the logging functions to use this function
  RETURN_IF_ERROR_CODE(readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempReading));
  if (tempReading>=80)
    overTemperatureDetected();
  else if (tempReading<=75)
    safeOperatingConditions();
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  thermal_mgr_event_t task;
  error_code_t errCode;
  float tempReading = 0.0;
  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, &task, 10) == pdTRUE)
    {
      if (task.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD)
      {
        //Imported the logging functions to use this function
        RETURN_IF_ERROR_CODE(readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempReading));
        addTemperatureTelemetry(tempReading);
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
