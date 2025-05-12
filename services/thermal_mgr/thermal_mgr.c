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
 * @brief Sends thermal manager event to queue
 *
 * @param event - Pointer to event to send
 * @return error_code_t - Success || error code
 */
error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  /* Send an event to the thermal manager queue */

  if (!event) return ERR_CODE_INVALID_ARG;
  if (!thermalMgrQueueHandle) return ERR_CODE_INVALID_STATE;
  if (!xQueueSend(thermalMgrQueueHandle, event, 0)) return ERR_CODE_QUEUE_FULL;

  return ERR_CODE_SUCCESS;
}

/**
 * @brief LM75BD OS interrupt -> enqueue handler event.
 */
void osHandlerLM75BD(void) {
  /* Implemented this function */
  thermal_mgr_event_t event = {.type = THERMAL_MGR_EVENT_HANDLE_OS};
  thermalMgrSendEvent(&event);
}

/**
 * @brief Thermal manager waits for queue event, then performs thermal logic.
 *
 * @param pvParameters - Pointer of LM75BD config struct.
 */
static void thermalMgr(void *pvParameters) {
  /* Implemented this task */
  if (!pvParameters) {
  	LOG_ERROR_CODE(ERR_CODE_INVALID_ARG);
  return;
  } else if (!thermalMgrQueueHandle) {
    LOG_ERROR_CODE(ERR_CODE_INVALID_STATE);
    return;
  }

  thermal_mgr_event_t event;
  error_code_t errCode;
  float temp = 0;

  //Struct data configeration copy
  lm75bd_config_t data = *(lm75bd_config_t *) pvParameters;
  uint8_t devAddr = data.devAddr;

  while (1) {
    //Blocked until recieved element from queue
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY)) { //pdTrue
      //Attempts to read current temp
      errCode = readTempLM75BD(devAddr, &temp);
      if (errCode != ERR_CODE_SUCCESS) {
        LOG_ERROR_CODE(errCode);
        continue;
      }
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) { //Case 1: normal temperature
        addTemperatureTelemetry(temp);
      } else if (event.type == THERMAL_MGR_EVENT_HANDLE_OS) { //Case 2: OS handle interupt event
        if (temp > TEMP_HYS) {
          overTemperatureDetected();
        } else {
          safeOperatingConditions();
        }
      } else {
        LOG_ERROR_CODE(ERR_CODE_INVALID_QUEUE_MSG);
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
