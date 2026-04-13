#include "controller.h"
#include "console.h"
#include "thermal_mgr.h"
#include "lm75bd.h"
#include "errors.h"
#include "i2c_io.h"
#include "logging.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* DO NOT MODIFY ANYTHING IN THIS FILE */

// This file contains the task that controls the test environment
// It handles the temperature sensor simulation.

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
  initI2C();
  initLogger();

  static lm75bd_config_t config = {0};
  config.devAddr = LM75BD_OBC_I2C_ADDR;
  config.osFaultQueueSize = 1;
  config.osPolarity = LM75BD_OS_POL_ACTIVE_LOW;
  config.osOperationMode = LM75BD_OS_OP_MODE_INT;
  config.devOperationMode = LM75BD_DEV_OP_MODE_NORMAL;

  // Use the sensor's default overtemperature and hysteresis thresholds
  config.overTempThresholdCelsius = 80.0f;
  config.hysteresisThresholdCelsius = 75.0f;

  const uint16_t overTempThresholdRegVal = (uint16_t)(config.overTempThresholdCelsius / 0.125f) << 5;
  const uint16_t hysteresisThresholdRegVal = (uint16_t)(config.hysteresisThresholdCelsius / 0.125f) << 5;

  // Initialize peripherals before other tasks are created
  lm75bdInit(&config);

  const uint16_t testTempRegSeq[] = {16000, 17000, 18944, 19000, 19500, 19700, 20000, 20736, 21000, 21500};
  const uint8_t tempRegSeqSize = sizeof(testTempRegSeq) / sizeof(testTempRegSeq[0]);
  uint8_t nextValIndex = 0;
  int seqDir = 1;

  setLm75bdNextTempRegVal(testTempRegSeq[0]);
  nextValIndex = 1;

  // Create thermal management task and pass it the sensor config
  initThermalSystemManager(&config);

  uint8_t overTempThreshExceeded = 0;

  const uint16_t testRunLimit = 4 * tempRegSeqSize;
  uint16_t testRunCount = 0;
  
  while (1) {

    if (testRunCount >= testRunLimit) {
      // We don't want the test environment to run forever
      printConsole("Exiting the test environment. Examine the output to determine if your implementation is correct.\n");
      exit(0);
    }

    thermal_mgr_event_t event;

    if (testTempRegSeq[nextValIndex] >= overTempThresholdRegVal && seqDir == 1 && !getOsActive() && !overTempThreshExceeded) {
      overTempThreshExceeded = 1;
      setOsActive(1);
      osHandlerLM75BD();
    } else if (testTempRegSeq[nextValIndex] <= hysteresisThresholdRegVal && seqDir == -1 && !getOsActive() && overTempThreshExceeded) {
      overTempThreshExceeded = 0;
      setOsActive(1);
      osHandlerLM75BD();
    } else {
      event.type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD;
      thermalMgrSendEvent(&event);
    }

    if (nextValIndex == 0) {
      seqDir = 1;
    } else if (nextValIndex == tempRegSeqSize - 1) {
      seqDir = -1;
    }

    nextValIndex = (nextValIndex + seqDir) % tempRegSeqSize;

    setLm75bdNextTempRegVal(testTempRegSeq[nextValIndex]);

    testRunCount++;

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
