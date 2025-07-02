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

  if(xQueueSend(thermalMgrQueueHandle,event,0) != pdTRUE){
    return errQUEUE_FULL; //this is a round about way of doing it as xQueueSend already gives this value?
  };

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {


  float temp = __FLT_MAX__; //for catching any read temp errors
  readTempLM75BD(LM75BD_OBC_I2C_ADDR,&temp);//will this block?(using the driver function I wrote)



  if( temp >  LM75BD_DEFAULT_HYST_THRESH){
    //interupt was called because we are over temp
    overTemperatureDetected();
  }else{//if the interrupt happened and we are not over temp, that means we just returned to regular temps
    safeOperatingConditions();
  }

}

static void thermalMgr(void *pvParameters) {

  //create buffer to read into(just of 1 element)
  thermal_mgr_event_t buffer; //create a buffer to read into

  while (1) {//infinite loop that should be broken out of at some point(interrupts)
  if(xQueueReceive(thermalMgrQueueHandle, &buffer, 0) == pdTRUE){//if we successfully read from queue


    if(buffer.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){//if the current element in the buffer is of the correct type
      float temp = __FLT_MAX__; //initialize temp var
      readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp); //pass in the temp variable we just made.
      addTemperatureTelemetry(temp);//send temp over to telemetry
    }
    
  };

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
