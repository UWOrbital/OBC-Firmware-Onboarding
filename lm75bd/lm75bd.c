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
  error_code_t errCode;

  //checks to ensure temp is not NULL
  if (temp == NULL){
    return ERR_CODE_INVALID_ARG;
  }

  //sets the pointer register to send data to
  const uint8_t ptrReg = 0x00;
  uint8_t sendBuf[1] = {ptrReg};
  RETURN_IF_ERROR_CODE(i2cSendTo(devAddr, sendBuf, 1));

  //reads data and stores in buffer
  uint8_t receiveBuf[2] = {0};
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(devAddr, receiveBuf, 2));

  //combines both bytes of data, and bitshift it to remove the irrelevant bits
  const uint8_t numIgnoredBits = 5;
  uint16_t tempData = ((receiveBuf[0] << 8) | receiveBuf[1]) >> numIgnoredBits;

  //checks if temperature is +/- and calculates temperature
  const uint16_t tempSign = 0x400;
  const uint16_t maskTempSign = 0x7FF;
  const float degCPerBit = 0.125f;
  if ((tempSign & tempData) == tempSign){
    //take two's complement and mask sign before converting to temperature
    *temp = -(((~tempData) + 1) & maskTempSign) * degCPerBit;
  } 
  else{
    *temp = tempData * degCPerBit;
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
