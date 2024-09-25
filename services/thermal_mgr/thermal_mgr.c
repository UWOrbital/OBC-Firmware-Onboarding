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
  
  if(event == NULL || thermalMgrQueueHandle == NULL){
    return ERR_CODE_INVALID_QUEUE_MSG;
  }
	
  if(xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) {
    return ERR_CODE_UNKNOWN;
  }
  // I am not sure when xQueueSend fails but probably ERR_CODE_QUEUE_FULL and ERR_CODE_INVALID_QUEUE_MSG work better
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  thermal_mgr_event_t event;
  // This struct only has one member
  event.type = THERMAL_MGR_EVENT_OS_INTERRUPT;

  xQueueSend(thermalMgrQueueHandle, &event, 0);
}

static void thermalMgr(void *pvParameters) {
  // Config will passed from initThermalSystemManager
  lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;

  while (1) {
    thermal_mgr_event_t event;
    error_code_t errCode;
    if(xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdPASS){
      // If Event is to Check Temperature
      if(event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
        float tempC = 0;
	LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &tempC));
	if(errCode != ERR_CODE_SUCCESS) continue;
	addTemperatureTelemetry(tempC);
      }
      // If Event is an OS Interupt
      else if(event.type == THERMAL_MGR_EVENT_OS_INTERRUPT){ // This was added to the enum in the thermal_mgr.h file
        // Implement: if temperature was recorded earlier then check that instead of calling a func
	float tempC = 0;
	LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &tempC));
        if(errCode != ERR_CODE_SUCCESS) continue;
	// Determining which interupt it was 
	if(tempC < config.hysteresisThresholdCelsius){
	  safeOperatingConditions();
	}
	else{
	  overTemperatureDetected();
	}
      }
      else{
        LOG_ERROR_CODE(ERR_CODE_INVALID_EVENT_RECEIVED);
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

