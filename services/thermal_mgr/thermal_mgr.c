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

#define TEMP_HYS 75U

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
  if (event == NULL) { return ERR_CODE_INVALID_ARG; } // event is null error
  if (thermalMgrQueueHandle == NULL) { return ERR_CODE_INVALID_STATE; } // queue handle is null error
  if (xQueueSend(thermalMgrQueueHandle, event, 0) != pdPASS) {
    // checks if this returns pdpass, if not then error queue full
    return ERR_CODE_QUEUE_FULL;
  }
  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  /* Implement this function */
  uint8_t devAddr = LM75BD_OBC_I2C_ADDR;
  thermal_mgr_event_t event = {THERMAL_MGR_EVENT_HANDLE_OS};
  error_code_t errCode;
  LOG_IF_ERROR_CODE(&event);
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  if (pvParameters == NULL) {
    LOG_ERROR_CODE(ERR_CODE_INVALID_ARG);
    return;
  }
  
  if (thermalMgrQueueHandle == NULL) { 
    LOG_ERROR_CODE(ERR_CODE_INVALID_STATE);
    return;
  }

  thermal_mgr_event_t event;
  float temp = 0;
  lm75bd_config_t data = *(lm75bd_config_t *) pvParameters;
  uint8_t devAddr = data.devAddr;

  while (1) {
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdPASS) {
      // received thermal_mgr_event_t
      // check its .type value
      // measure current temp.
      error_code_t result = readTempLM75BD(devAddr, &temp); 
      // check if return success, if not then log errorcode
      if (result != ERR_CODE_SUCCESS) {
        LOG_ERROR_CODE(result);
        continue;
      }
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
        addTemperatureTelemetry(temp);
      } else if (event.type == THERMAL_MGR_EVENT_HANDLE_OS) {
        if (temp > TEMP_HYS) {
          overTemperatureDetected(); // temp > Thys, over temp.
        } else {
          safeOperatingConditions(); // otherwise, normal temp.
        }
      } else { 
        LOG_ERROR_CODE(ERR_CODE_INVALID_QUEUE_MSG); // wrong event type in queue
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
