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

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {

  // Check that event pointer is not NULL
  if (event == NULL)
    return ERR_CODE_NULL_POINTER;

  // Check that the Queue exists by checking the the handle is not NULL
  if (thermalMgrQueueHandle == NULL)
    return ERR_CODE_QUEUE_MISSING;

  /* Send an event to the thermal manager queue */
  if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdTRUE)
    return ERR_CODE_QUEUE_FULL;

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t event;
  event.type = THERMAL_MGR_EVENT_INTERRUPT;
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */

  // Get config structure and initialize event and temperature params
  lm75bd_config_t config = *(lm75bd_config_t *) pvParameters;

  while (1) {

    error_code_t err;
    thermal_mgr_event_t event;

    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdTRUE){
      
      float temperature;

      switch (event.type) {
        case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
          err = readTempLM75BD(config.devAddr, &temperature);

          if (err != ERR_CODE_SUCCESS){
            LOG_ERROR_CODE(err);
            break;
          }

          addTemperatureTelemetry(temperature);
          break;

        case THERMAL_MGR_EVENT_INTERRUPT:
          err = readTempLM75BD(config.devAddr, &temperature);

          if (err != ERR_CODE_SUCCESS){
            LOG_ERROR_CODE(err);
            break;
          }

          if (temperature > config.hysteresisThresholdCelsius){
            overTemperatureDetected();
          } else {
            safeOperatingConditions();
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
