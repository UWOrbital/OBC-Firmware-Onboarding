#include "i2c_io.h"
#include "errors.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>
#include <os_atomic.h>

#include <stdint.h>
#include <string.h>

// The mutex timeout for the I2C bus
#define I2C_MUTEX_TIMEOUT pdMS_TO_TICKS(500)

static SemaphoreHandle_t i2cMutex;
static StaticSemaphore_t i2cMutexBuffer;

// For automated testing
static uint16_t lm75bdNextTempRegVal;

void setLm75bdNextTempRegVal(uint16_t val) {
  // This function is only called from the test suite (controller task)
  portENTER_CRITICAL();
  lm75bdNextTempRegVal = val;
  portEXIT_CRITICAL();
}

static uint16_t getLm75bdNextTempRegVal(void) {
  portENTER_CRITICAL();
  uint16_t val = lm75bdNextTempRegVal;
  portEXIT_CRITICAL();
  return val;
}

void initI2C(void) {
  memset(&i2cMutexBuffer, 0, sizeof(i2cMutexBuffer));
  i2cMutex = xSemaphoreCreateMutexStatic(&i2cMutexBuffer);
}

error_code_t i2cSendTo(uint8_t sAddr, uint8_t *buf, uint16_t numBytes) {
  if (buf == NULL || numBytes < 1) return ERR_CODE_INVALID_ARG;

  if (i2cMutex == NULL) return ERR_CODE_INVALID_STATE;

  if (xSemaphoreTake(i2cMutex, I2C_MUTEX_TIMEOUT) != pdTRUE) {
    return ERR_CODE_MUTEX_TIMEOUT;
  }

  /* Mock the transmit */

  xSemaphoreGive(i2cMutex);  // Won't fail because the mutex is taken correctly
  return ERR_CODE_SUCCESS;
}

error_code_t i2cReceiveFrom(uint8_t sAddr, uint8_t *buf, uint16_t numBytes) {
  if (buf == NULL || numBytes < 1) return ERR_CODE_INVALID_ARG;

  if (i2cMutex == NULL) return ERR_CODE_INVALID_STATE;

  if (xSemaphoreTake(i2cMutex, I2C_MUTEX_TIMEOUT) != pdTRUE) {
    return ERR_CODE_MUTEX_TIMEOUT;
  }

  /* Mock the receive using getLm75bdNextTempRegVal() */

  xSemaphoreGive(i2cMutex);  // Won't fail because the mutex is taken correctly
  return ERR_CODE_SUCCESS;
}
