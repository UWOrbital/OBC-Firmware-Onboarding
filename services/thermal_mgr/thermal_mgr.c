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

#define OT_THRESHOLD 85 
#define HYS_THRESHOLD 75

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
  if(thermalMgrQueueHandle == NULL) return ERR_CODE_QUEUE_FULL;
  if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) return ERR_CODE_INVALID_QUEUE_MSG/*failed case*/;
  // check to make sure the queue has been created 
  return ERR_CODE_SUCCESS;
}


// you currently have 1 change requests left to make 


void osHandlerLM75BD(void) { 
  /* Implement this function */
  thermal_mgr_event_t osInterruptEvent;
  osInterruptEvent.type = THERMAL_MGR_EVENT_OS_INTERRUPT; // Set the event type to the newly defined type
  thermalMgrSendEvent(&osInterruptEvent); // Send the event to the queue
}



static void thermalMgr(void *pvParameters) { 
  /* Implement this task */
  while (1) {
    // Question , what do I declare this as? 
    thermal_mgr_event_t receivedEvent;

    // Task should only perform an action if it receives an event through the thermal manager queue
		if (xQueueReceive(thermalMgrQueueHandle, &receivedEvent, portMAX_DELAY) == pdPASS) { // set the addresses for this 
			 // Check if it's the temperature measure event
			 if (receivedEvent.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
			     // collect temperature data using drive fcn 
			     float temperature; 
					 error_code_t errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temperature);
			     // Send it as telemetry
           // only add telemetry temperature if the reading was a success
			     if(errCode == ERR_CODE_SUCCESS) addTemperatureTelemetry(temperature);
			  }
        if (receivedEvent.type == THERMAL_MGR_EVENT_OS_INTERRUPT) {
			     // Kemi: Note that, if you decide to do this, you’ll need to define a new event type to send to the thermal manager queue.
          // Team Lead -> u dont necessarily need to read the temperature in the OShandler, u just need a way to communicate that an 
          // interrupt has happened so that u can read the temperature and deal with it from outside to OS handler
          float temperature; 
          error_code_t errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temperature); // Use your readTempLM75BD function
           if(errCode == ERR_CODE_SUCCESS){
              if (temperature > HYS_THRESHOLD) {
                  // Temperature is above or equal to the overtemperature thresholdF
                  overTemperatureDetected();
              } else if (temperature <= HYS_THRESHOLD) {
                  // Temperature is below or equal to the hysteresis threshold
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

