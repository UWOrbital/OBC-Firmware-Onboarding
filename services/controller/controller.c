#include "controller.h"
#include "console.h"
#include "thermal_mgr.h"
#include "lm75bd.h"
#include "errors.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <stdint.h>
#include <string.h>

#define CONTROLLER_STACK_SIZE 256U

static TaskHandle_t controllerTaskHandle;
static StaticTask_t controllerTaskBuffer;
static StackType_t controllerTaskStack[CONTROLLER_STACK_SIZE];

static void controller(void *pvParameters);

void initController(void) {
  memset(&controllerTaskBuffer, 0, sizeof(controllerTaskBuffer));
  memset(controllerTaskStack, 0, sizeof(controllerTaskStack));

  controllerTaskHandle = xTaskCreateStatic(
    controller, "controller", CONTROLLER_STACK_SIZE,
    NULL, 1, controllerTaskStack, &controllerTaskBuffer);   
}

static void controller(void *pvParameters) {
  // Initialize the mutex that protects the console output
  initConsole();

  static lm75bd_config_t config = {0};
  config.devAddr = LM75BD_OBC_I2C_ADDR;
  config.osFaultQueueSize = 1;
  config.osPolarity = LM75BD_OS_POL_ACTIVE_LOW;
  config.osOperationMode = LM75BD_OS_OP_MODE_INT;
  config.devOperationMode = LM75BD_DEV_OP_MODE_NORMAL;

  // Use the sensor's default overtemperature and hysteresis thresholds
  config.overTempThresholdCelsius = 75.0f;
  config.hysteresisThresholdCelsius = 80.0f;

  lm75bdInit(&config);

  // Create thermal management task and pass it the sensor config
  initThermalSystemManager(&config);

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
