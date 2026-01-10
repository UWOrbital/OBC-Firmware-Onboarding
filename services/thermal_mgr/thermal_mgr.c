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
  //checks to ensure event is not NULL
  if (event == NULL){
    return ERR_CODE_INVALID_ARG;
  }

  if(xQueueSend(thermalMgrQueueHandle, event, 0) == errQUEUE_FULL){
    return errQUEUE_FULL;
  } else{
    return ERR_CODE_SUCCESS;
  }
}

void osHandlerLM75BD(void) {
  //sends event to thermal manager that there was an interrupt
  thermal_mgr_event_t osEvent = {.type = OS_HANDLER_INTERRUPT};

  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(thermalMgrQueueHandle, &osEvent, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Thermal manager RTOS task
 *
 * @param pvParameters - Task argument expected to point to an lm75bd_config_t
 * @return void
 */
static void thermalMgr(void *pvParameters) {
  error_code_t errCode;

  //checks to ensure pvParameters is not NULL
  if (pvParameters == NULL){
    return;
  }

  thermal_mgr_event_t event;
  //copies the configuration
  lm75bd_config_t config = *(lm75bd_config_t *) pvParameters;

  //while loop that runs indefintely checking for events
  while (1) {
    //checks to see if an event was received
    if (xQueueReceive(thermalMgrQueueHandle, &event ,portMAX_DELAY) == pdPASS){
      //reads temperature
      if(event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD){
        float temp = 0.0f;
        LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temp));
        if (errCode == ERR_CODE_SUCCESS){
          addTemperatureTelemetry(temp);
        }
      }
      //reads temperature and checks if overtemperature or safe
      else if(event.type == OS_HANDLER_INTERRUPT){
          float temp = 0.0f;
          LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temp));
          if (errCode == ERR_CODE_SUCCESS){
          if(temp >= config.overTempThresholdCelsius){
            overTemperatureDetected();
          } 
          else if (temp <= config.hysteresisThresholdCelsius){
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
