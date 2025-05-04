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

/**
 * @brief Sends event to thermal manager queue
 * 
 * @param event - Pointer to thermal manager event to send
 * @return ERR_CODE_SUCCESS if the event is successfully sent
 */
error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  /* Send an event to the thermal manager queue */
  if(event == NULL) return ERR_CODE_INVALID_ARG;
  if(thermalMgrQueueHandle == NULL) return ERR_CODE_INVALID_ARG;

  xQueueSend(thermalMgrQueueHandle, event, 0);
  return ERR_CODE_SUCCESS;
}

/**
 * @brief OS interrupt handler for LM75B temperature sensor
 * 
 */
void osHandlerLM75BD(void) {
  /* Implement this function */
  thermal_mgr_event_t event = {0};
  thermalMgrSendEvent(&event);
}

/**
 * @brief Thermal management task for temperature events
 * 
 * @param pvParameters - Pointer to parameters passed to task
 */
static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  while (1) {
    thermal_mgr_event_t data = {0};
    if(xQueueReceive(thermalMgrQueueHandle, &data, portMAX_DELAY) == pdTRUE && data.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
      float temperature;
      error_code_t result = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &temperature);
      if(result != ERR_CODE_SUCCESS) LOG_ERROR_CODE(result);
      else addTemperatureTelemetry(temperature);

      if(temperature > LM75BD_DEFAULT_HYST_THRESH){ // only below T hys is safe
        overTemperatureDetected();
      }
      else safeOperatingConditions();
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
