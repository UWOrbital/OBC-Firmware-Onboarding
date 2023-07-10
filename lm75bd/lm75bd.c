#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_TEMP 0x00U  /* Temperature Register (R) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */

error_code_t lm75bdInit(lm75bd_config_t *config) {
  error_code_t errCode;

  if (config == NULL) return ERR_CODE_INVALID_ARG;

  errCode = writeConfigLM75BD(config->devAddr, config->osFaultQueueSize, config->osPolarity,
                                         config->osOperationMode, config->devOperationMode);
  
  if (errCode != ERR_CODE_SUCCESS) return errCode;

  // Assume default overtemperature and hysteresis thresholds

  return ERR_CODE_SUCCESS;
}

error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  /* Implement this driver function */
  uint8_t txBuff[1] = {LM75BD_REG_TEMP};
  uint8_t rxBuff[2] = {0};

  i2cSendTo(devAddr, txBuff, 1);
  i2cReceiveFrom(devAddr, rxBuff, 2);

  uint16_t regVal = (rxBuff[0] << 8) | rxBuff[1];
  int16_t sRegVal = regVal >> 5;
      
  if (sRegVal & (1 << 10)) {
    sRegVal |= (0x3F << 11); // Set bits 11-15 to 1 if negative      
  }
  
  *temp = sRegVal * 0.125f;
  
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

error_code_t osHandlerLM75BD(uint8_t devAddr) {
  /* Implement this driver function */

  return ERR_CODE_SUCCESS;
}
