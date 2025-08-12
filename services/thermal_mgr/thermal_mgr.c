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
  /* Send an event to the thermal manager queue */
  if (event == NULL) return ERR_CODE_INVALID_ARG;

  if (thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;

  if (xQueueSend(thermalMgrQueueHandle, event, (TickType_t) 10) != pdPASS){
    // Failed to post message even after 10 ticks
    return ERR_CODE_INVALID_STATE;
  }
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t event;
  event.type = THERMAL_MGR_EVENT_OS_HANDLER;
  thermalMgrSendEvent(&event); 
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;
  while (1) {
    thermal_mgr_event_t tme;

    // Indefinitely block until something is received from the queue
    if (xQueueReceive(thermalMgrQueueHandle, &tme, (TickType_t) portMAX_DELAY) == pdPASS){
      float tempC = 0.0f;
      error_code_t errCode;
      LOG_IF_ERROR_CODE(
          readTempLM75BD(config.devAddr, &tempC);
      );

      // If readTemp fails, move to the next iteration
      if (errCode != ERR_CODE_SUCCESS){
          LOG_ERROR("Temperature read from device failed. Continuing.");
          continue;
      }

      switch (tme.type){
        case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
            addTemperatureTelemetry(tempC);
            break;
        case THERMAL_MGR_EVENT_OS_HANDLER:
            if (tempC > config.overTempThresholdCelsius){
              overTemperatureDetected();
            }
            else if (tempC < config.hysteresisThresholdCelsius) {
              safeOperatingConditions();
            }
            break;
        default:
            LOG_ERROR("Event received from queue has unsupported type.");
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
