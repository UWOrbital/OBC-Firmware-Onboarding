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
  if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) return ERR_CODE_QUEUE_FULL;
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Function Implemented */
  thermal_mgr_event_t event;
  event.type = THERMAL_MGR_EVENT_OS;
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {
  /* Task Implemented */
  lm75bd_config_t config = *(lm75bd_config_t *)pvParameters; /* Cast the parameter to the correct type */
  thermal_mgr_event_t event;

  /* Do nothing if queue is NULL*/
  if (thermalMgrQueueHandle == NULL) {
    LOG_ERROR_CODE(ERR_CODE_INVALID_STATE);
    vTaskDelete(NULL);
  }

  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdPASS) {
        switch (event.type) {
          /* Measure temperature and handle accordingly */
          case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD: {
            float tempC;
            error_code_t errCode = readTempLM75BD(config.devAddr, &tempC);
            if (errCode != ERR_CODE_SUCCESS) {
              LOG_ERROR_CODE(errCode); 
            } else {
              addTemperatureTelemetry(tempC);
            }
            break;
          }
          
          case THERMAL_MGR_EVENT_OS: {
            float tempC;
            error_code_t errCode = readTempLM75BD(config.devAddr, &tempC);
            if (errCode != ERR_CODE_SUCCESS) {
              LOG_ERROR_CODE(errCode);
            } else {
              if (tempC >= config.hysteresisThresholdCelsius) {
                overTemperatureDetected();
              } else {
                safeOperatingConditions();
              }
            }
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

