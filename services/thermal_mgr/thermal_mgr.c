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
  if (event == NULL) return ERR_CODE_INVALID_ARG;
  if (thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;
  /* Send an event to the thermal manager queue */
  //block time is 0 because function is being called in an ISR
  if (xQueueSend(thermalMgrQueueHandle, event, 0) == pdTRUE) return ERR_CODE_SUCCESS;
  return ERR_CODE_QUEUE_FULL;
}

void osHandlerLM75BD(void) {
  //send an event everytime this handler is called
  thermal_mgr_event_t event;
  event.type = THERMAL_MGR_EVENT_OS_INTERRUPT;

  thermalMgrSendEvent(&event);

  

}


static void thermalMgr(void *pvParameters) {
  
  /* Implement this task */
  while (1) {
    thermal_mgr_event_t event;
    float temp;
    error_code_t errCode;

    if ((thermalMgrQueueHandle != NULL)){
      if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdTRUE) {

  
        if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
          errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp);
          if (errCode != ERR_CODE_SUCCESS){
            printConsole("%i\n", errCode);
            continue;
          } 
          addTemperatureTelemetry(temp);
        } else if (event.type == THERMAL_MGR_EVENT_OS_INTERRUPT) {
          errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp);
          if (errCode != ERR_CODE_SUCCESS){
            printConsole("%i\n", errCode);
            continue;
          } 
          if (temp > 75) {
            overTemperatureDetected();
          } else {
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
