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
  if(event == NULL) return ERR_CODE_INVALID_ARG;

  if(thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;

  if(xQueueSend(thermalMgrQueueHandle, (void *) event, TEMPERATURE_WAIT_TIME) != pdTRUE) {
    return ERR_CODE_QUEUE_FULL;
  }

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  thermal_mgr_event_t event = {.type = THERMAL_MGR_EVENT_OS_HANDLE};
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(thermalMgrQueueHandle, (void *) &event, &xHigherPriorityTaskWoken);
  taskYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void thermalMgr(void *pvParameters) {
  // Set variables and get config for lm75bd
  lm75bd_config_t *config = (lm75bd_config_t *) pvParameters;
  
  while (1) {
    thermal_mgr_event_t event;
    if (xQueueReceive(thermalMgrQueueHandle, (void *) &event, TEMPERATURE_RECEIVE_TIME) == pdTRUE) {
      
      // Part 2 of challenge
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
        float temp; 
        if(readTempLM75BD(config->devAddr, &temp) != ERR_CODE_SUCCESS) 
          continue;
        addTemperatureTelemetry(temp);

      // Part 3 of challenge 
      } else if(event.type == THERMAL_MGR_EVENT_OS_HANDLE) {
        float temp; 
        if(readTempLM75BD(config->devAddr, &temp) != ERR_CODE_SUCCESS) 
          continue;

        if(temp > config->hysteresisThresholdCelsius) {
          overTemperatureDetected();
        } else {
          safeOperatingConditions();
        }

      // Failure
      } else {
        
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
