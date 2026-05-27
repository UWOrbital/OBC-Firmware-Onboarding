#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include "logging.h"

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

  if (xQueueSend(thermalMgrQueueHandle, event, pdMS_TO_TICKS(100)) == pdPASS)
    return ERR_CODE_SUCCESS;
  return ERR_CODE_QUEUE_FULL;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  // better to call overTemperatureDetected and safeOperatingConditions functions in thermalManagement task
  // call overTempeartureDetected if you exceeded T_th and call safe OperatingConditions when we're dropped back down below T_hys
  // Now this sends a fault and the actual implementation is done with task context instead of in the ISR so its actually debuggable
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  thermal_mgr_event_t event = { .type = THERMAL_MGR_EVENT_FAULT };
  xQueueSendFromISR(thermalMgrQueueHandle, &event, &pxHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  // get the devAddr directly from the sensor config struct pointer, which is passed from pvParameters, set when the task is created?
  lm75bd_config_t config = *(lm75bd_config_t *) pvParameters;
  // this is defined in services/controller/controller.c, done to reduce magic numbers
  config.hysteresisThresholdCelsius;  // cache it before ISR can fire
  thermal_mgr_event_t event;
  error_code_t errCode;

  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdPASS) {
      // Handle event
            float temperature = 0;
            switch (event.type) {
                case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
                    // measure
                    LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temperature));
                    // send as telemetry
                    addTemperatureTelemetry(temperature);
                    break;
                // this is the event the ISR will fire instead of doing everything in the ISR
                case THERMAL_MGR_EVENT_FAULT:
                    // implementation moved here instead of ISR      
                    LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temperature));
                    addTemperatureTelemetry(temperature);
                    if (temperature > config.hysteresisThresholdCelsius) {
                      overTemperatureDetected();
                    } else {
                      safeOperatingConditions();
                    }
                    break;
                default:
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

