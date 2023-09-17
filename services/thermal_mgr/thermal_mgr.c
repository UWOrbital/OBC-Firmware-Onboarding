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
  BaseType_t send = xQueueSend(thermalMgrQueueHandle, event, 0); // 0 ticks to wait 
  if (send != pdPASS) return ERR_CODE_INVALID_QUEUE_MSG/*failed case*/;
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) { 
  /* Implement this function */
  float* temperature;
  // Question: a, I using the correct devAddr 
    error_code_t errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, temperature); // Use your readTempLM75BD function

    if (temperature > HYS_THRESHOLD) {
        // Temperature is above or equal to the overtemperature threshold
        overTemperatureDetected();
    } else if (temperature <= HYS_THRESHOLD) {
        // Temperature is below or equal to the hysteresis threshold
        safeOperatingConditions(); 
    }
}

static void thermalMgr(void *pvParameters) { // Kemi: fix this also 
  /* Implement this task */
  while (1) {
    // Question , what do I declare this as? 
    thermal_mgr_event_t receivedEvent;

    // Task should only perform an action if it receives an event through the thermal manager queue
		if (xQueueReceive(thermalMgrQueueHandle, &receivedEvent, portMAX_DELAY) == pdPASS) { // set the addresses for this 
			 // Check if it's the temperature measure event
       // Kemi: Obtain type how
			 if (receivedEvent.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
			     // collect temperature data using drive fcn 
			     float* temperature; 
           // Kemi: Obtain devAddr Question, is this the correct devAddr
					 error_code_t errCode = readTempLM75BD(LM75BD_OBC_I2C_ADDR, temperature);
					 if (errCode != ERR_CODE_SUCCESS) /*do something*/;
			     // Send it as telemetry
			     addTemperatureTelemetry(*temperature);
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



//  Questions 
//  1. I do not know if I am passing the right address to the readTempLM75BD() function for the devAddr, I think I am reading from the wrong devAddr 
//  2. I am struggling with the bitwise conversions and the celcius conversion 
//  3. thermalMgrSendEvent
//  4. I need help understanding how to test 