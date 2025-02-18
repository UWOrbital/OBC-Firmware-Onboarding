#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"
#include "logging.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */

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
  /* Implement this driver function */
  // Read the current temperature from the LM75BD
  // - select the sensor's internal temperature register using the pointer register

  error_code_t errCode;
  uint8_t tempReg = 0x00;
  uint8_t tempBuff[2] = {0};  // This is 2 bytes because the temperature is 9 bits

  // - send the register address to the sensor
  errCode = i2cSendTo(devAddr, &tempReg, 1);
  if (errCode != ERR_CODE_SUCCESS) return errCode;

  // - read the temperature value from the sensor
  errCode = i2cReceiveFrom(devAddr, tempBuff, 2);
  if (errCode != ERR_CODE_SUCCESS) return errCode;

  // In MSB, the 8th bit 0 if positive, 1 if negative
  // In LSB, bits 1-5 is ignored, bits 6-8 is the temperature value along with the MSB

  // Combine the MSB and LSB to get the temperature value
  int16_t tempVal = (int16_t)(tempBuff[0] << 8) | tempBuff[1];

  // Check if the temperature is negative
  if (tempVal & 0x8000) {
    // If the temperature is negative, convert it to a negative value
    tempVal = ~tempVal + 1;
    *temp = -(float)(tempVal >> 5) * 0.125f;
  } else {
    // Convert to Celsius
    *temp = (float)(tempVal >> 5) * 0.125f;
  }
  
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
