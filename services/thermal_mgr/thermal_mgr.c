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

void initThermalSystemManager(lm75bd_config_t *config)
{
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

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event)
{
  /* Send an event to the thermal manager queue */
  /* Send an event to the thermal manager queue */

  // check to make sure the queue has been created already
  if (event == NULL)
    return ERR_CODE_INVALID_QUEUE_MSG;
  if (thermalMgrQueueHandle == NULL)
  {
    return ERR_CODE_INVALID_STATE; // Return an error code if the queue hasn't been created
  }

  if (xQueueSend(thermalMgrQueueHandle, (void *)event, (TickType_t)0) == pdTRUE)
  {
    return ERR_CODE_SUCCESS;
  }
  else
    return ERR_CODE_QUEUE_FULL; // If it doesn't send, then queue is full
}

void osHandlerLM75BD(void)
{
  /* Implement this function */
  thermal_mgr_event_t interruptEvent;
  interruptEvent.type = THERMAL_MGR_EVENT_INTERRUPT;
  thermalMgrSendEvent(&interruptEvent);
}

static void thermalMgr(void *pvParameters)
{
  /* Implement this task */

  while (1)
  {
    thermal_mgr_event_t receivedData;
    float currTemp = 0;
    error_code_t checker;
    if (xQueueReceive(thermalMgrQueueHandle, &receivedData, portMAX_DELAY) == pdTRUE)
    {
      switch (receivedData.type)
      {
      case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
        checker = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &currTemp);
        if (checker != ERR_CODE_SUCCESS)
        {
          // Convert the error code to a string using to_string
          string errorString = to_string(static_cast<int>(checker));

          // Print the error string to the console
          printConsole("Error code: %s", errorString.c_str());
          continue;
        }
        addTemperatureTelemetry(currTemp);
        break;
      case THERMAL_MGR_EVENT_INTERRUPT:
        checker = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &currTemp);
        if (checker != ERR_CODE_SUCCESS)
        {
          // Convert the error code to a string using to_string
          string errorString = to_string(static_cast<int>(checker));
          // Print the error string to the console
          printConsole("Error code: %s", errorString.c_str());
          continue;
        }
        if (currTemp >= 80)
        {
          overTemperatureDetected();
        }
        else
        {
          safeOperatingConditions();
        }
        break;
      default:
        printConsole("Queue data type is unsupported.");
        break;
      }
    }
  }
}

void addTemperatureTelemetry(float tempC)
{
  printConsole("Temperature telemetry: %f deg C\n", tempC);
}

void overTemperatureDetected(void)
{
  printConsole("Over temperature detected!\n");
}

void safeOperatingConditions(void)
{
  printConsole("Returned to safe operating conditions!\n");
}
