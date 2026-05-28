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
  BaseType_t sent = xQueueSend(thermalMgrQueueHandle, event, 10);

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t OSEvent;
  OSEvent.type = THERMAL_MGR_EVENT_OS_INTERRUPT;
  BaseType_t higherPriorityTaskWoken = pdFALSE;

  BaseType_t interruptSent = xQueueSendToFrontFromISR(thermalMgrQueueHandle, &OSEvent, &higherPriorityTaskWoken);
}

static void thermalMgr(void *pvParameters) {
  lm75bd_config_t config = *(lm75bd_config_t *) pvParameters; // create a copy of incoming parameters

  while (1) {
    thermal_mgr_event_t receivedFromQueue;
    BaseType_t received = xQueueReceive(thermalMgrQueueHandle, &receivedFromQueue, 10); // receieve from queue
    // NOTE: WHAT SHOULD THE # OF TICKS BE? double check w admin before submit

    if (received == pdPASS) {
      if (receivedFromQueue.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
        float temp = 0.0f;
        error_code_t err = readTempLM75BD(config.devAddr, &temp);

        if (err == ERR_CODE_SUCCESS) {
          addTemperatureTelemetry(temp);
        }
      } else if (receivedFromQueue.type == THERMAL_MGR_EVENT_OS_INTERRUPT) {
        float temp = 0.0f;
        error_code_t err = readTempLM75BD(config.devAddr, &temp);
        
        if (err == ERR_CODE_SUCCESS) {
          if (temp > config.hysteresisThresholdCelsius) {
            overTemperatureDetected();
          } else {
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
