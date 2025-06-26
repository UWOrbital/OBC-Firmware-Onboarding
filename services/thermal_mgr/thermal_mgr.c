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

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {//unsure how many ticks to wait.
  /* Send an event to the thermal manager queue */

  if(xQueueSend(thermalMgrQueueHandle,event,0) != pdTRUE){//unsure what needs to be done if this fails, no error code for FREERTOS errQUEUE_EMPTY
    return errQUEUE_FULL; //this is a round about way of doing it as xQueueSend already gives this value, should fix later
  };//if we could not send to queue

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */

  /* Notes
  -Interrupts when going above Tth or below Thys. So we don't know weather its been called after going over Tth or below Thys.
  -We must reset it by reading a register, but how to read a register without using the driver function??
  -Appears that this is not beign called as I never see the printconsole message
  */
  printConsole("In Interrupt osHandler");

  float temp = 0;
  readTempLM75BD(LM75BD_OBC_I2C_ADDR,&temp);//will this block?

  if( temp >  LM75BD_DEFAULT_HYST_THRESH){
    overTemperatureDetected();
  }else{
    safeOperatingConditions();
  }

}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */

  /* Notes
  -reveive event thru thermal manager queue 
  -check if it's type is set to THERMAL_MGR_EVENT_MEASURE_TEMP_CMD
  -Not doing any error handling as no return value?
  */
  uint8_t i = 0;//for keeping track of where we are in the buffer.
  //create buffer to read into
  thermal_mgr_event_t buffer[THERMAL_MGR_QUEUE_LENGTH]; //create a buffer to read into, not sure how to initialize

  while (1) {//infinite loop that should be broken out of at some point
  xQueueReceive(thermalMgrQueueHandle, &buffer, 0);//removed error checking


  if(buffer[i].type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){//if the current element in the buffer is of the correct type
    float temp = 0; //initialize temp var
    readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temp); //pass in the temp variable we just made.
    addTemperatureTelemetry(temp);//send temp over
  }
  i++;

  if(i == THERMAL_MGR_QUEUE_LENGTH){
    i = 0;//reset buffer if it has reached the end
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
