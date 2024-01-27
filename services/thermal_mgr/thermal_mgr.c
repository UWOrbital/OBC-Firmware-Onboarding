#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include <string.h>

#include "logging.h"

#define THERMAL_MGR_STACK_SIZE 256U
#define I2C_TEMP_OT_THRESH 80.0f
#define I2C_TEMP_HYST_THRESH 75.0f

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
  
  if(event == NULL) return ERR_CODE_INVALID_ARG;
  if(thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;
  if(xQueueSend(thermalMgrQueueHandle, event, 0) == errQUEUE_FULL) return ERR_CODE_QUEUE_FULL;
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t event = {.type = THERMAL_MGR_EVENT_INTERRUPT};
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  while (1) {
    error_code_t errCode;
    thermal_mgr_event_t tempEvent;
    float tempVal;


    if (xQueueReceive(thermalMgrQueueHandle, &tempEvent, portMAX_DELAY) == pdTRUE){
      if (tempEvent.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD)
      {
        errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempVal);
        if(errCode == ERR_CODE_SUCCESS) {
          addTemperatureTelemetry(tempVal);
        }
      }
      else if (tempEvent.type == THERMAL_MGR_EVENT_INTERRUPT) {
        errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempVal);
        if (errCode == ERR_CODE_SUCCESS) {
          if (tempVal >= I2C_TEMP_OT_THRESH) {
            overTemperatureDetected();
          }
          if (tempVal <= I2C_TEMP_HYST_THRESH) {
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
