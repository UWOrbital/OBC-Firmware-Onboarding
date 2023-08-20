#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */
#define LM75BD_REG_TEMP 0x00U
#define WRITE_BYTES 1U
#define READ_BYTES 2U
#define TEMPERATURE_CONVERSION_FACTOR 0.125f

error_code_t lm75bdInit(lm75bd_config_t *config) {
  error_code_t errCode;

  if (config == NULL) return ERR_CODE_INVALID_ARG;

  errCode = writeConfigLM75BD(config->devAddr, config->osFaultQueueSize, config->osPolarity,
                                         config->osOperationMode, config->devOperationMode);
  
  if (errCode != ERR_CODE_SUCCESS) return errCode;

  // Assume that the overtemperature and hysteresis thresholds are already set
  // Hysteresis: 75 degrees Celsius
  // Overtemperature: 80 degrees Celsius

  return ERR_CODE_SUCCESS;
}

error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  /* Implement this driver function */
  // Initialize local variables
  if (temp == NULL) return ERR_CODE_INVALID_ARG;

  uint8_t buffer[READ_BYTES] = {0};
  uint8_t tempReg = LM75BD_REG_TEMP;

  // Set temperature registry
  error_code_t error = i2cSendTo(devAddr, &tempReg, WRITE_BYTES);
  if (error != ERR_CODE_SUCCESS) return error;

  // read from temperature registry
  error = i2cReceiveFrom(devAddr, buffer, READ_BYTES);
  if (error != ERR_CODE_SUCCESS) return error;

  // Combine 
  int16_t regVal = (buffer[0] << 8 | buffer[1]);

  // Calculate temperature
  *temp = (float) (regVal >> 5) * TEMPERATURE_CONVERSION_FACTOR;

  return ERR_CODE_SUCCESS;
}

#define CONF_WRITE_BUFF_SIZE 2U
error_code_t writeConfigLM75BD(uint8_t devAddr, uint8_t osFaultQueueSize, uint8_t osPolarity,
                                   uint8_t osOperationMode, uint8_t devOperationMode) {
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
  if (errCode != ERR_CODE_SUCCESS) return errCode;

  return ERR_CODE_SUCCESS;
}
