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
static volatile BaseType_t pendingOsEvent = pdFALSE;

static void thermalMgr(void *pvParameters);

void initThermalSystemManager(lm75bd_config_t *config) {
  configASSERT(config != NULL);

  memset(&thermalMgrTaskBuffer, 0, sizeof(thermalMgrTaskBuffer));
  memset(thermalMgrTaskStack, 0, sizeof(thermalMgrTaskStack));
  
  memset(&thermalMgrQueueBuffer, 0, sizeof(thermalMgrQueueBuffer));
  memset(thermalMgrQueueStorageArea, 0, sizeof(thermalMgrQueueStorageArea));

  thermalMgrQueueHandle = xQueueCreateStatic(
    THERMAL_MGR_QUEUE_LENGTH, THERMAL_MGR_QUEUE_ITEM_SIZE,
    thermalMgrQueueStorageArea, &thermalMgrQueueBuffer);
  configASSERT(thermalMgrQueueHandle != NULL);

  pendingOsEvent = pdFALSE;
  thermalMgrTaskHandle = xTaskCreateStatic(
    thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE,
    config, 1, thermalMgrTaskStack, &thermalMgrTaskBuffer);
  configASSERT(thermalMgrTaskHandle != NULL);

}

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  if (event == NULL) return ERR_CODE_INVALID_ARG;
  if (thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_STATE;
  if (event->type != THERMAL_MGR_EVENT_MEASURE_TEMP_CMD &&
      event->type != THERMAL_MGR_EVENT_OS_INTERRUPT) {
    return ERR_CODE_INVALID_QUEUE_MSG;
  }

  if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) {
    return ERR_CODE_QUEUE_FULL;
  }

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  thermal_mgr_event_t event = {.type = THERMAL_MGR_EVENT_OS_INTERRUPT};

  // This simulator calls the handler from the controller task
  if (thermalMgrQueueHandle == NULL) return;
  if (xQueueSendToFront(thermalMgrQueueHandle, &event, 0) != pdPASS) {
    taskENTER_CRITICAL();
    pendingOsEvent = pdTRUE;
    taskEXIT_CRITICAL();
  }
}

/**
 * @brief Read the sensor and handle queued telemetry and watchdog events
 * @param pvParameters Sensor configuration supplied when the task is created
 */
static void thermalMgr(void *pvParameters) {
  lm75bd_config_t *config = (lm75bd_config_t *)pvParameters;
  configASSERT(config != NULL);

  while (1) {
    thermal_mgr_event_t event;
    taskENTER_CRITICAL();
    BaseType_t retryOsEvent = pendingOsEvent;
    pendingOsEvent = pdFALSE;
    taskEXIT_CRITICAL();

    if (retryOsEvent) {
      event.type = THERMAL_MGR_EVENT_OS_INTERRUPT;
    } else if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) != pdPASS) {
      continue;
    }

    float tempC;
    error_code_t errCode = readTempLM75BD(config->devAddr, &tempC);
    if (errCode != ERR_CODE_SUCCESS) {
      LOG_ERROR_CODE(errCode);
      continue;
    }

    switch (event.type) {
      case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
        addTemperatureTelemetry(tempC);
        break;
      case THERMAL_MGR_EVENT_OS_INTERRUPT:
        if (tempC > config->hysteresisThresholdCelsius) {
          overTemperatureDetected();
        } else {
          safeOperatingConditions();
        }
        break;
      default:
        LOG_ERROR_CODE(ERR_CODE_INVALID_QUEUE_MSG);
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
