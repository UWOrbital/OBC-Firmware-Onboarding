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


#define TEMPERATURE_HYSTERESIS_THRESHOLD 75.0 // Defining hysteresis threshold as a macro here

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
  BaseType_t checkQueueSend;
  if (event == NULL) {
    return ERR_CODE_INVALID_ARG;
  }
  checkQueueSend = xQueueSend(thermalMgrQueueHandle, (void *) event, (TickType_t) 10);
  if (checkQueueSend == pdFAIL) {
    return ERR_CODE_INVALID_ARG;
  }
  return ERR_CODE_SUCCESS;
  
}

void osHandlerLM75BD(void) {
  
  /* Implement this function */
  thermal_mgr_event_t interrupt;
  interrupt.type = THERMAL_MGR_EVENT_TYPE_INTERRUPT;
  thermalMgrSendEvent(&interrupt); 
  
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  float tempData;
  error_code_t readTempCheck;
  thermal_mgr_event_t eventTypeCheck; 

  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, (void *) &eventTypeCheck , (TickType_t) 10) == pdTRUE) {
      switch (eventTypeCheck.type) {
        case (THERMAL_MGR_EVENT_MEASURE_TEMP_CMD):
          if (readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempData) == ERR_CODE_SUCCESS) {
            addTemperatureTelemetry(tempData);
          }
          break;
        
        case (THERMAL_MGR_EVENT_TYPE_INTERRUPT):
          if (readTempLM75BD(LM75BD_OBC_I2C_ADDR, &tempData) == ERR_CODE_SUCCESS) {
            if (tempData > TEMPERATURE_HYSTERESIS_THRESHOLD) {
              overTemperatureDetected();
            }
            else {
              safeOperatingConditions();
            }
          }
          break;    
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
