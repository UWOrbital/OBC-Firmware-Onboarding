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

//event buffer to check event type
static thermal_mgr_event_t messageBuffer;

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
  if (event -> type != THERMAL_MGR_EVENT_MEASURE_TEMP_CMD && event -> type != THERMAL_MGR_EVENT_OS){
      return ERR_CODE_INVALID_QUEUE_MSG;
    }
  /* Send an event to the thermal manager queue */
  if (xQueueSend(thermalMgrQueueHandle, event, (TickType_t) 0) == pdPASS){
    return ERR_CODE_SUCCESS; 
  }

  else{
    //If queue is full
    return ERR_CODE_QUEUE_FULL;
  }

  
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  
  //OS threshold was crossed or temp dropped below hysteresis 
  thermalMgrSendEvent(&os);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  while (1) {
    // variable to store temperature
    float temp;
    //receive event from queue
    if(xQueueReceive(thermalMgrQueueHandle, &messageBuffer, portMAX_DELAY) == pdPASS){
      error_code_t errCode;
      //update temp variable/reset OS 
      LOG_IF_ERROR_CODE(readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp));
      if (messageBuffer.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
      addTemperatureTelemetry(temp);
      }

      else if(messageBuffer.type == THERMAL_MGR_EVENT_OS){
         if (temp > LM75BD_DEFAULT_OT_THRESH){
          overTemperatureDetected();
         }

         else if(temp < LM75BD_DEFAULT_HYST_THRESH){
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
