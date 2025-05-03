#include "lm75bd.h"
#include "errors.h"
#include "i2c_io.h"
#include "logging.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
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
  // Define errCode for error macros
  error_code_t errCode;

  // The pointer register address for the temperature sensor
  uint8_t tempReg = 0;

  // The blob of memory that will take two 8 bit values that represent the
  // reading
  uint8_t readBuff[2] = {0};

  // Send the temperature sensor address to the device first to select the
  // temperature sensor
  RETURN_IF_ERROR_CODE(i2cSendTo(devAddr, &tempReg, 1));
  // Recieve data from the temperature sensor
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(devAddr, readBuff, 2));

  // Start converting by taking the array, converting it to a large 16 bit
  // signed integer and deleting off the last five bits as they are useless
  int16_t tempVal = (int16_t)((readBuff[0] << 8) | readBuff[1]) >> 5;

  printf("Val: %d", tempVal);
  // Check if the first bit is a 1 or 0 and follow datasheet instructions
  // accordingly
  if (readBuff[0] & 0x80) {
    // The signed integer will store 2s complement but that won't work well with
    // floats. So we take the original unsigned binary representation by using
    // two's complement again. We convert the unsigned value to a float and then
    // multiply by the required constant and negative which makes the float
    // encoding correct!
    *temp = (float)(~(tempVal) + 1) * -0.125;
  } else {
    // If the number was positive to begin with, then it is already represented
    // as an unsigned and no coverting is needed.
    *temp = (float)(tempVal) * 0.125;
  }

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
