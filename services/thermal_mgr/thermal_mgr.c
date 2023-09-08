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
  /* Returns appropriate error code according to queue status*/
  if (event == NULL)
  {
    return ERR_CODE_INVALID_ARG;
  }
  if (xQueueSend(thermalMgrQueueHandle, (void *)event, portMAX_DELAY) == errQUEUE_FULL)
  {
    return ERR_CODE_QUEUE_FULL;
  }

  return ERR_CODE_SUCCESS;
}

void osHandlerLM75BD(void)
{
  /* Implement this function */

  /* Declare event of interrupt type and send to the queue*/
  thermal_mgr_event_t interrupt;
  interrupt.type = THERMAL_MGR_EVENT_INTERRUPT;
  thermalMgrSendEvent(&interrupt);
}

static void thermalMgr(void *pvParameters)
{
  /* Implement this task */
  while (1)
  {
    /* Variable declarations*/
    thermal_mgr_event_t currentItem;
    error_code_t status;
    float currentTemp = 0;

    /* Current item is only processed if it was successfully recieved from the queue*/
    if (xQueueReceive(thermalMgrQueueHandle, &currentItem, portMAX_DELAY) == pdTRUE)
    {
      /* Switch statement to perform type comparison on current item*/
      switch (currentItem.type)
      {
      /* In case the current item of normal/default type, record the current temperature and, if successful, print current temp to console*/
      case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:
        status = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &currentTemp);
        if (status != ERR_CODE_SUCCESS)
        {
          printConsole("%d\n", status);
          continue;
        }
        addTemperatureTelemetry(currentTemp);

      /* If the current item is an OS interrupt, check if the current temperature is past hysteresis conditions and call the appropriate function*/
      case THERMAL_MGR_EVENT_INTERRUPT:
        status = readTempLM75BD(LM75BD_OBC_I2C_ADDR, &currentTemp);
        if (status != ERR_CODE_SUCCESS)
        {
          printConsole("%d\n", status);
          continue;
        }

        if (currentTemp > 75) // 75 degrees is temperature hysteresis
        {
          overTemperatureDetected();
        }
        else
        {
          safeOperatingConditions();
        }
        break;
      default:
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
