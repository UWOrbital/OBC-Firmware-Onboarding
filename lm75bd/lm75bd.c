#include "lm75bd.h"
#include "console.h"
#include "errors.h"
#include "i2c_io.h"
#include "logging.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U /* Configuration Register (R/W) */

error_code_t lm75bdInit(lm75bd_config_t *config) {
  error_code_t errCode;

  if (config == NULL)
    return ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(writeConfigLM75BD(
      config->devAddr, config->osFaultQueueSize, config->osPolarity,
      config->osOperationMode, config->devOperationMode));

  // Assume that the overtemperature and hysteresis thresholds are already set
  // Hysteresis: 75 degrees Celsius
  // Overtemperature: 80 degrees Celsius

  return ERR_CODE_SUCCESS;
}

error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  // check if temp is allocated
  if (!temp)
    return ERR_CODE_NULL_ARG;

  // set pointer register to temperature
  uint8_t buf[1] = {LM75BD_OBC_TEMP_REG};
  error_code_t errCode;
  RETURN_IF_ERROR_CODE(i2cSendTo(devAddr, buf, 1));
  // read data from sensor
  uint8_t tempBytes[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(devAddr, tempBytes, 2));

  // convert bytes to temperature
  int16_t combined = ((tempBytes[0] << 8) | tempBytes[1]);
  combined = combined >> 5;
  *temp = combined * 0.125;
  return ERR_CODE_SUCCESS;
}

#define CONF_WRITE_BUFF_SIZE 2U
error_code_t writeConfigLM75BD(uint8_t devAddr, uint8_t osFaultQueueSize,
                               uint8_t osPolarity, uint8_t osOperationMode,
                               uint8_t devOperationMode) {
  error_code_t errCode;

  // Stores the register address and data to be written
  // 0: Register address
  // 1: Data
  uint8_t buff[CONF_WRITE_BUFF_SIZE] = {0};

  buff[0] = LM75BD_REG_CONF;

  uint8_t osFaltQueueRegData = 0;
  switch (osFaultQueueSize) {
  case 1:
    osFaltQueueRegData = 0;
    break;
  case 2:
    osFaltQueueRegData = 1;
    break;
  case 4:
    osFaltQueueRegData = 2;
    break;
  case 6:
    osFaltQueueRegData = 3;
    break;
  default:
    return ERR_CODE_INVALID_ARG;
  }

  buff[1] |= (osFaltQueueRegData << 3);
  buff[1] |= (osPolarity << 2);
  buff[1] |= (osOperationMode << 1);
  buff[1] |= devOperationMode;

  errCode = i2cSendTo(LM75BD_OBC_I2C_ADDR, buff, CONF_WRITE_BUFF_SIZE);
  if (errCode != ERR_CODE_SUCCESS)
    return errCode;

  return ERR_CODE_SUCCESS;
}
