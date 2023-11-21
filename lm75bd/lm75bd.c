#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"
#include "logging.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */
#define LM75BD_TEMP_REG 0x00U // Temperature register address


error_code_t lm75bdInit(lm75bd_config_t *config) {
  error_code_t errCode;

  if (config == NULL) return ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(writeConfigLM75BD(config->devAddr, config->osFaultQueueSize, config->osPolarity,
                                         config->osOperationMode, config->devOperationMode));

  // Assume that the overtemperature and hysteresis thresholds are already set
  // Hysteresis: 75 degrees Celsius
  // Overtemperature: 80 degrees Celsius

  return ERR_CODE_SUCCESS;
}


error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  error_code_t errCode; // Variable to store error codes.
  
  if (temp == NULL) {
    LOG_ERROR_CODE(ERR_CODE_INVALID_ARG);
    return ERR_CODE_INVALID_ARG;
  }

  uint8_t command[1] = {LM75BD_TEMP_REG};

  // Set the register address in the device 
  RETURN_IF_ERROR_CODE(i2cSendTo(devAddr, command, 1));

  // Buffer to receive data from LM75BD
  uint8_t dataBuffer[2] = {0};

  // Receive 2 bytes of temperature data and store them in dataBuffer
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(devAddr, dataBuffer, 2));

  // Combine the bytes into a 16-bit integer for conversion
  int16_t rawTemp = (int16_t)(dataBuffer[0] << 8) | dataBuffer[1];

  // Convert the raw temperature data to degrees Celsius according to the datasheet specification.
  *temp = (float)(rawTemp >> 5) * 0.125;
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
