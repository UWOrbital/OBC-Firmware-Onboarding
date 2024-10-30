#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"
#include "logging.h"

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

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

  uint8_t temp_pointer_byte = 0x00U; // selects temp register at 00 LSB
  uint8_t *send_buf = &temp_pointer_byte;

  i2cSendTo(devAddr, send_buf, 1);

  uint8_t receive_buf[2];
  i2cReceiveFrom(devAddr, receive_buf, 2);
  uint16_t MSByte = (uint16_t) receive_buf[0];
  uint16_t LSByte = (uint16_t) receive_buf[1];

  int16_t temp_reg_val = (MSByte << 3) + (LSByte >> 5);
  // if 11 bit is set, sign is -ve, so convert the 11 bit val to its equivalent 16 bit val
  // otherwise, sign is +ve, and the 11 bit val will be the same in 16 bits
  if (temp_reg_val & 0x0400U) {
    // by simply setting everything above the 11 bits to 1 (top 5 bits here)
    temp_reg_val |= 0xF800U;
  }
  printf("Temperature Register Value: %i\n", temp_reg_val);
  
  float final_temp = ((float) temp_reg_val) * 0.125f;
  printf("Final Temperature: %f\n", final_temp);
  *temp = final_temp;
  
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
