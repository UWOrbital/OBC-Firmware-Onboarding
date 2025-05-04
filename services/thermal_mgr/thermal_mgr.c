#include "thermal_mgr.h"
#include "console.h"
#include "errors.h"
#include "lm75bd.h"
#include "logging.h"
#include "os_portmacro.h"
#include "os_projdefs.h"

#include <FreeRTOS.h>
#include <os_queue.h>
#include <os_task.h>

#include <string.h>

#define THERMAL_MGR_STACK_SIZE 256U

static TaskHandle_t thermalMgrTaskHandle;
static StaticTask_t thermalMgrTaskBuffer;
static StackType_t thermalMgrTaskStack[THERMAL_MGR_STACK_SIZE];

#define THERMAL_MGR_QUEUE_LENGTH 10U
#define THERMAL_MGR_QUEUE_ITEM_SIZE sizeof(thermal_mgr_event_t)

static QueueHandle_t thermalMgrQueueHandle;
static StaticQueue_t thermalMgrQueueBuffer;
static uint8_t thermalMgrQueueStorageArea[THERMAL_MGR_QUEUE_LENGTH *
                                          THERMAL_MGR_QUEUE_ITEM_SIZE];

static void thermalMgr(void *pvParameters);

void initThermalSystemManager(lm75bd_config_t *config) {
  memset(&thermalMgrTaskBuffer, 0, sizeof(thermalMgrTaskBuffer));
  memset(thermalMgrTaskStack, 0, sizeof(thermalMgrTaskStack));

  thermalMgrTaskHandle =
      xTaskCreateStatic(thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE,
                        config, 1, thermalMgrTaskStack, &thermalMgrTaskBuffer);

  memset(&thermalMgrQueueBuffer, 0, sizeof(thermalMgrQueueBuffer));
  memset(thermalMgrQueueStorageArea, 0, sizeof(thermalMgrQueueStorageArea));

  thermalMgrQueueHandle =
      xQueueCreateStatic(THERMAL_MGR_QUEUE_LENGTH, THERMAL_MGR_QUEUE_ITEM_SIZE,
                         thermalMgrQueueStorageArea, &thermalMgrQueueBuffer);
}

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
  /* Send an event to the thermal manager queue */

  // We check if the event was succesfully queued and return accordingly
  if (xQueueSend(thermalMgrQueueHandle, event, 0) == pdPASS) {
    return ERR_CODE_SUCCESS;
    // We check if the queue is full and return accordingly
  } else if (xQueueIsQueueFullFromISR(thermalMgrQueueHandle) == pdFALSE) {
    return ERR_CODE_QUEUE_FULL;
    // If none of the previous cases occured, then we do not know the error and
    // we return an ERR_CODE_UNKNOWN
  } else {
    return ERR_CODE_UNKNOWN;
  }
}

void osHandlerLM75BD(void) {
  // This interupt is called everytime the temperature goes over the
  // overtemperature limit or under the hysteresis temperature

  // Define the error code for the macros
  error_code_t errCode;

  // We create a new type of event to send into the queue since we want to
  // minimize the execution time of this function, so let's offload the work to
  // our thermal manager function
  thermal_mgr_event_t osInterrupt;

  // From the header we define a new type of event to look for in our thermal
  // manager
  osInterrupt.type = THERMAL_MGR_EVENT_HYS_TEMP;

  // With some error handling, we send the event into the queue
  LOG_IF_ERROR_CODE(thermalMgrSendEvent(&osInterrupt));
}

static void thermalMgr(void *pvParameters) {
  /* Implement this task */
  while (1) {
    // Define the error code for the macros
    error_code_t errCode;

    // Define an event that will be passed into the recieve from queue function
    thermal_mgr_event_t event;

    // Recieve from queue and check if it was a success
    if (xQueueReceive(thermalMgrQueueHandle, &event, portMAX_DELAY) == pdPASS) {

      // From the parameters extract the sensor configuration
      lm75bd_config_t config = *(lm75bd_config_t *)pvParameters;

      // Define a variable to hold the temperature reading from the sensor
      float temp = 0;

      // With some error, use the function from the sensor driver to read the
      // temperature with the address passed in from the config struct
      LOG_IF_ERROR_CODE(readTempLM75BD(config.devAddr, &temp));

      // Check if this is a simple read event
      if (event.type == THERMAL_MGR_EVENT_MEASURE_TEMP_CMD) {
        addTemperatureTelemetry(temp);

        // Check if the event is for hysteresis or overtemperature
      } else if (event.type == THERMAL_MGR_EVENT_HYS_TEMP) {

        // Check for overtemperature and call the appropriate function
        if (temp > 80) {
          overTemperatureDetected();

          // Check for hysteresis and call the appropriate function
        } else if (temp < 75) {
          safeOperatingConditions();
        }
      }
    } else {
      printConsole("Message was not succesfully recieved from queue");
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
