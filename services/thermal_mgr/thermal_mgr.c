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

#define MAX_MS_WAIT 20
error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  if (!event)                 { return ERR_CODE_INVALID_ARG; }
  if (!thermalMgrQueueHandle) { return ERR_CODE_INVALID_STATE; }

  /* Send an event to the thermal manager queue */
  if (xQueueSend(thermalMgrQueueHandle, (void *)event,
      portTICK_PERIOD_MS*MAX_MS_WAIT) == pdTrue) {
    // Successful send
    return ERR_CODE_SUCCESS;
  } else {
    return ERR_CODE_QUEUE_FULL;
  }
}

error_code_t thermalMgrSendEventISR(thermal_mgr_event_t *event) {
  /* Send an event to the thermal manager queue */
  if (!event)                 { return ERR_CODE_INVALID_ARG; }
  if (!thermalMgrQueueHandle) { return ERR_CODE_INVALID_STATE; }

  if (xQueueSendFromISR(thermalMgrQueueHandle, (void *)event, NULL) == pdTRUE) {
    return ERR_CODE_SUCCESS;
  } else {
    return ERR_CODE_QUEUE_FULL;
  }
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  // Use new thermalMgr event for non-blocking.
  thermal_mgr_event_t evt = {0};
  evt.type = THERMAL_MGR_EVENT_OVER_TEMP_CMD;
  thermalMgrSendEventISR(&evt);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  lm75bd_config_t config = *(lm75bd_config_t *) pvParameters;
  float temp = {0};
  error_code_t err_code = {0};

  while (1) {
    thermal_mgr_event_t temp_evt = {0};
    xQueueReceive(thermalMgrQueueHandle, (void *)&temp_evt, portTICK_PERIOD_MS*MAX_MS_WAIT);

    switch (temp_evt.type) {
    case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
      /* Measure temperature */
      err_code = readTempLM75BD(config.devAddr, &temp);
      if (err_code == ERR_CODE_SUCCESS) {
        addTemperatureTelemetry(temp); // ambiguous error case
      }
      break;
    case THERMAL_MGR_EVENT_OVER_TEMP_CMD:
      err_code = readTempLM75BD(config.devAddr, &temp);
      if (err_code == ERR_CODE_SUCCESS) {
        if (temp <= config.hysteresisThresholdCelsius) {
          // safe temperature
          safeOperatingConditions();
        } else {
          overTemperatureDetected();
        }
      }
      break;
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
