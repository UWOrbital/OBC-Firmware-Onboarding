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
  if (event == NULL){
    return ERR_CODE_INVALID_ARG;
  }
  if(thermalMgrQueueHandle == NULL){
    return ERR_CODE_INVALID_STATE;
  }
  if (xQueueSend(thermalMgrQueueHandle, event, pdMS_TO_TICKS(10)) == errQUEUE_FULL){
    return ERR_CODE_QUEUE_FULL;
  }
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  thermal_mgr_event_t event;
  event.type = THERMAL_MGR_EVENT_OS_INTERRUPT;
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters){
  lm75bd_config_t config = *(lm75bd_config_t*)pvParameters;
  while (1) {
    thermal_mgr_event_t eventBuf;
    if (xQueueReceive(thermalMgrQueueHandle, &eventBuf, 0) != errQUEUE_EMPTY){
      if (eventBuf.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
        float temp;
        error_code_t errCode = readTempLM75BD(config.devAddr, &temp);
        if(errCode == ERR_CODE_SUCCESS){
          addTemperatureTelemetry(temp);
        }
        else {
          LOG_ERROR_CODE(errCode);
        }
      }
      else if (eventBuf.type == THERMAL_MGR_EVENT_OS_INTERRUPT){
        float temp;
        error_code_t errCode = readTempLM75BD(config.devAddr, &temp);
        if (errCode == ERR_CODE_SUCCESS){
          if (temp > config.hysteresisThresholdCelsius){
            overTemperatureDetected();
          }
          else {
            safeOperatingConditions();
          }
        }
        else {
          error_code_t errCode = ERR_CODE_INVALID_STATE;
          LOG_ERROR_CODE(errCode);
        }
      }
      else {
        error_code_t errCode;
        LOG_ERROR_CODE(ERR_CODE_INVALID_STATE);
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
