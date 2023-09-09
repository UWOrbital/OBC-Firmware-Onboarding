#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */

/* Address of Temp Register */
#define TEMP_REGISTER 0x00

/* R/W Buffers */
#define WRITE_BUFFER _SIZE_T 1
#define READ_BUFFER  _SIZE_T 2

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

  uint8_t pointerBuffer = TEMP_REGISTER;

  error_code_t send_code = i2cSendTo(devAddr, &pointerBuffer, WRITE_BUFFER);

  if (send_code != ERR_CODE_SUCCESS){
    return send_code;
  }
  
  uint8_t data[READ_BUFFER] = {0};
  error_code_t receive_code = i2cReceiveFrom(devAddr, data, READ_BUFFER);

  if (receive_code != ERR_CODE_SUCCESS){
    return receive_code;
  }

  uint16_t raw_temp = (data[0] << 8) | data[1]; // Get 11 most significant bits
  raw_temp = raw_temp >> 5; // Shift down 11 MSB

  if (raw_temp & 0x0400){ // If 11th bit is 1, compute negative
    raw_temp = ((~raw_temp)+1) & 0x07FF;
    *temp = -raw_temp * 0.125f;
  }
  else{
    *temp = raw_temp * 0.125f;
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
