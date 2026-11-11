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

  if (event == NULL || thermalMgrQueueHandle == NULL) {
    return ERR_CODE_INVALID_ARG;
  }

  BaseType_t status;

  if (event -> type == THERMAL_MGR_EVENT_OS_INTERRUPT) {
    // if we are in an interrupt call - logic that was previously in osHandler
    BaseType_t higherPriority = pdFALSE;
    status = xQueueSendToFrontFromISR(thermalMgrQueueHandle, event, &higherPriority);

    if (status == pdPASS) {
      portYIELD_FROM_ISR(higherPriority);
    }
  } else {
    // sending regular task, with no blocking
    status = xQueueSend(thermalMgrQueueHandle, event, 0); 
  }

  if (status != pdPASS) {
    return ERR_CODE_QUEUE_FULL;
  }

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void) {
  thermal_mgr_event_t event = { .type = THERMAL_MGR_EVENT_OS_INTERRUPT};
  thermalMgrSendEvent(&event);
}

static void thermalMgr(void *pvParameters) {

  if (pvParameters == NULL) {
    // must use vTaskDelete at end of task fucntion not just return (forcing job to be finsihed must delete itself)
    vTaskDelete(NULL);
  }
  
  lm75bd_config_t data =  *(lm75bd_config_t *) pvParameters;
  thermal_mgr_event_t recv_buff;

  while (1) {
    if ((xQueueReceive(thermalMgrQueueHandle, &recv_buff, portMAX_DELAY) == pdPASS)) {
      float temp;

      error_code_t err = readTempLM75BD(data.devAddr, &temp);

      if (err != ERR_CODE_SUCCESS) {
        LOG_ERROR_CODE(err);
        continue; // dont want to use the garbage in temp
      }

      switch (recv_buff.type) {
        case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
          addTemperatureTelemetry(temp);
          break;

        case THERMAL_MGR_EVENT_OS_INTERRUPT:
          if (temp > LM75BD_DEFAULT_HYST_THRESH) {
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
