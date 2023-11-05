#include "i2c_io.h"
#include "errors.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>
#include <os_atomic.h>

#include <stdint.h>
#include <string.h>

/* DO NOT MODIFY ANYTHING IN THIS FILE */

// The mutex timeout for the I2C bus
#define I2C_MUTEX_TIMEOUT pdMS_TO_TICKS(500)

static SemaphoreHandle_t i2cMutex;
static StaticSemaphore_t i2cMutexBuffer;

// Test environment variables
static uint16_t lm75bdNextTempRegVal = 0;
static uint8_t lastTxBuff[2] = {0};
static uint8_t isOsActive = 0;

void initI2C(void) {
  memset(&i2cMutexBuffer, 0, sizeof(i2cMutexBuffer));
  i2cMutex = xSemaphoreCreateMutexStatic(&i2cMutexBuffer);
}

/* MOCKED I2C Functions. DO NOT MODIFY */

error_code_t i2cSendTo(uint8_t sAddr, uint8_t *buf, uint16_t numBytes) {
  if (buf == NULL || numBytes < 1) return ERR_CODE_INVALID_ARG;

  if (i2cMutex == NULL) return ERR_CODE_INVALID_STATE;

  if (xSemaphoreTake(i2cMutex, I2C_MUTEX_TIMEOUT) != pdTRUE) {
    return ERR_CODE_MUTEX_TIMEOUT;
  }

  lastTxBuff[0] = buf[0];

  if (numBytes > 1) {
    lastTxBuff[1] = buf[1];
  }

  xSemaphoreGive(i2cMutex);  // Won't fail because the mutex is taken correctly

  return ERR_CODE_SUCCESS;
}

error_code_t i2cReceiveFrom(uint8_t sAddr, uint8_t *buf, uint16_t numBytes) {
  if (buf == NULL || numBytes < 1) return ERR_CODE_INVALID_ARG;

  if (i2cMutex == NULL) return ERR_CODE_INVALID_STATE;

  if (xSemaphoreTake(i2cMutex, I2C_MUTEX_TIMEOUT) != pdTRUE) {
    return ERR_CODE_MUTEX_TIMEOUT;
  }

  uint16_t nextTemp = getLm75bdNextTempRegVal();

  switch (lastTxBuff[0]) {
    case 0:
      buf[0] = (nextTemp >> 8) & 0xFF;
      buf[1] = nextTemp & 0xFF;
      break;
    default:
      for (int i = 0; i < numBytes; i++) {
        buf[i] = 0;
      }
  }

  setOsActive(0);

  xSemaphoreGive(i2cMutex);  // Won't fail because the mutex is taken correctly
  return ERR_CODE_SUCCESS;
}

/* TEST ENVIRONMENT FUNCTIONS */
void setOsActive(uint8_t val) {
  portENTER_CRITICAL();
  isOsActive = val;
  portEXIT_CRITICAL();
}

uint8_t getOsActive(void) {
  portENTER_CRITICAL();
  uint8_t isActive = isOsActive;
  portENTER_CRITICAL();
  return isActive;
}

void setLm75bdNextTempRegVal(uint16_t val) {
  // This function is only called from the test suite (controller task)
  portENTER_CRITICAL();
  lm75bdNextTempRegVal = val;
  portEXIT_CRITICAL();
}

uint16_t getLm75bdNextTempRegVal(void) {
  portENTER_CRITICAL();
  uint16_t val = lm75bdNextTempRegVal;
  portEXIT_CRITICAL();
  return val;
}
