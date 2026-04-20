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

  if (event==NULL){
    return ERR_CODE_INVALID_ARG;
  }

  if (thermalMgrQueueHandle==NULL){
    return ERR_CODE_INVALID_QUEUE_MSG;  
  }

  
  if ((xQueueSend(thermalMgrQueueHandle, &event, 1))==errQUEUE_FULL){
    return ERR_CODE_QUEUE_FULL;
  }


  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  //Read
  //Decide
  thermal_mgr_event_t event;
  event.type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD;
  thermalMgrSendEvent(&event);
  
  
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  while (1) {
    thermal_mgr_event_t event_result;
    
    xQueueReceive(thermalMgrQueueHandle, &event_result, portMAX_DELAY);    

    if (event_result.type==THERMAL_MGR_EVENT_MEASURE_TEMP_CMD||event_result.type==THERMAL_MGR_EVENT_MEASURE_EVAL_TEMP_CMD){
      float temperature;
      error_code_t read_Temp_Result = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temperature);
      if (read_Temp_Result!=ERR_CODE_SUCCESS){
        char char_Temp_Result[1];

        sprintf(char_Temp_Result, "%c", read_Temp_Result);
        printConsole(char_Temp_Result);
        continue;
      }
      addTemperatureTelemetry(temperature);
      if (event_result.type==THERMAL_MGR_EVENT_MEASURE_EVAL_TEMP_CMD){
        if (temperature>75.0){
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
