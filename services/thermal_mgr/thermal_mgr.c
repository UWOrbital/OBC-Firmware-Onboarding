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

/*THERMAL MANAGER SEND EVENT IMPLEMENATION FW ONBOARDING*/

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {  
  if (event == NULL) { 
    return ERR_CODE_INVALID_ARG;  
  } 
  else if (thermalMgrQueueHandle == NULL){
    return ERR_CODE_INVALID_STATE;
  }
  else if (xQueueSend(thermalMgrQueueHandle, event, 0) == pdPASS){ //send the event to the queue
    return ERR_CODE_SUCCESS;
  }
  else 
    return ERR_CODE_QUEUE_FULL;
}


/*OS HANDLER IMPLEMENTATION FW ONBOARDING*/

void osHandlerLM75BD(void) {
  thermal_mgr_event_t fault_event; //create the fault event
  fault_event.type = OS_FAULT_EVENT; //set the type to send to the queue
  thermalMgrSendEvent(&fault_event); 
}


/*THERMAL MANAGER IMPLMENTATION FW ONBOARDING*/

static void thermalMgr(void *pvParameters) {
  while (1) {
    error_code_t errCode;
    float check_temp = 0.0;
    thermal_mgr_event_t check_event; 

    if(xQueueReceive(thermalMgrQueueHandle, &check_event, portMAX_DELAY) == pdTRUE){ //receive the event from the queue
      if (check_event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) { //check if the event is the measure temperature one
        errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &check_temp);
          if (errCode != ERR_CODE_SUCCESS) { //check if the temperature reading was done
            LOG_ERROR_CODE(errCode);
          }
          else {
            addTemperatureTelemetry(check_temp);
          }
      }
      else if (check_event.type == OS_FAULT_EVENT) { //check if the event is the fault one  
        errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &check_temp);
          if (errCode != ERR_CODE_SUCCESS) { //check if the temperature reading was done
            LOG_ERROR_CODE(errCode);
          }
          else if (check_temp > 75.0) {
            overTemperatureDetected(); 
          } 
          else {
            safeOperatingConditions(); 
          }
      }
    }  
    else {
        LOG_ERROR_CODE(ERR_CODE_INVALID_QUEUE_MSG);
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
