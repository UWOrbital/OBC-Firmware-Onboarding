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

  error_code_t errCode;

  uint8_t pointerRegToTempRegBuf[1] = {0};
  RETURN_IF_ERROR_CODE(i2cSendTo(devAddr, pointerRegToTempRegBuf, 1));

  uint8_t tempData[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(devAddr, tempData, 2));

  uint8_t D10Bit = (tempData[0] & 0x80) >> 7; // Extract the most significant bit in the MSByte
  uint16_t tempVal = 0;

  // Cast MSByte to uint16_t and left shift by 3 bits to make room for the 3 most significant bits in the LSByte
  uint16_t tempMSByte = ((uint16_t)tempData[0] << 3); 

  // Cast LSByte to uint16_t and right shift by 5 bits to remove unneeded bits (extract the relavant bits)
  uint16_t tempLSByte = (uint16_t)((tempData[1] >> 5));

  // Combine relavant bits in MSByte and LSByte to get the 11 bits for temperature value 
  tempVal = tempMSByte | tempLSByte;

  if(D10Bit == 0){ // positive temperature
    *temp = tempVal * 0.125;
  }else if(D10Bit == 1){ // negative temperature
    // Get the 2s complement of the temperature before converting to degrees °C 
    tempVal = (~tempVal) & 0x07FF; // NOT temperature bits then AND the bits with 0x07FF to remove the irrelavant 5 most significant bits in the uint16_t
    *temp = -(float)((tempVal + 1) * 0.125);
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
