#include "controller.h"
#include "console.h"
#include "thermal_mgr.h"
#include "lm75bd.h"
#include "errors.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_timer.h>

#include <stdint.h>
#include <string.h>

#define CONTROLLER_STACK_SIZE 256U

static TaskHandle_t controllerTaskHandle;
static StaticTask_t controllerTaskBuffer;
static StackType_t controllerTaskStack[CONTROLLER_STACK_SIZE];

#define OVERTEMP_INT_PERIOD pdMS_TO_TICKS(3000)

static TimerHandle_t controllerTimerHandle;
static StaticTimer_t controllerTimerBuffer;

static void controller(void *pvParameters);
static void controllerTimerCallback(TimerHandle_t xTimer);

void initController(void) {
  memset(&controllerTaskBuffer, 0, sizeof(controllerTaskBuffer));
  memset(controllerTaskStack, 0, sizeof(controllerTaskStack));

  controllerTaskHandle = xTaskCreateStatic(
    controller, "controller", CONTROLLER_STACK_SIZE,
    NULL, 1, controllerTaskStack, &controllerTaskBuffer);   

  memset(&controllerTimerBuffer, 0, sizeof(controllerTimerBuffer));
  
  controllerTimerHandle = xTimerCreateStatic(
    "controllerTimer", OVERTEMP_INT_PERIOD, pdTRUE, NULL, controllerTimerCallback,
    &controllerTimerBuffer); 
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

  // Initialize peripherals before other tasks are created
  lm75bdInit(&config);

  // Create thermal management task and pass it the sensor config
  initThermalSystemManager(&config);

  xTimerStart(controllerTimerHandle, OVERTEMP_INT_PERIOD);

  while (1) {
    thermal_mgr_event_t event;
    event.type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD;
    thermalMgrSendEvent(&event);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

// For automated testing; defined in sys\i2c\i2c_io.c
extern void setLm75bdNextTempRegVal(uint16_t val);

static void controllerTimerCallback(TimerHandle_t xTimer) {
  setLm75bdNextTempRegVal(0x0000); // TODO: Set this to a value that tests the overtemperature handling
  osHandlerLM75BD(LM75BD_OBC_I2C_ADDR);
}
