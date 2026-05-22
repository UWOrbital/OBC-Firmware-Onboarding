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
  // need to access pointer register 000000 (00h)(temp register)
  // 4 bytes = 16 bits
  //uint8_t used b/c I2C works in 8 bit sequences
  uint8_t i2cBuffer[2];
  uint8_t pointerBuf = 0; // this should tell the pointer register to send from temperature register
  // tell the slave sensor via i2c you want to get data (idk how many bytes are needed for i2c handshake)
  i2cSendTo(devAddr, &pointerBuf, 1);

  //should read the 2 bytes (16 bits) form the temperature register, and ignore the last 5 (this function handles stop condition)
  i2cReceiveFrom(devAddr, i2cBuffer, 2);
  // now i2cBuffer[0] has first 8 bits, and [1] has last 8 bits
  // in a uint16_t, shift the high byte up by 8 bits since its the first 8 bits and attach the last 8 with bitwise "or"
  uint16_t temperature_binary = i2cBuffer[0] << 8 | i2cBuffer[1];
  // To filter out, you know the last 5 are 0/useless, so just shift all the 11 valuable bits to the left to get the value you want!!
  // Check the sign of the MSB to see if its +ve or -ve

  // bit mask with MSB to see if negative bit set or not
  if (temperature_binary & (1 << 15)){
    // then right shift to filter out unneeded bits at bottom
    temperature_binary = temperature_binary >> 5;
    // TODO: now sign extend, why again?
    temperature_binary |= 0xF800; // bitwise OR with 1111 1000 0000 0000, extending negative sign
    //(from datasheet) since negative, C = -(twos compliment of temp data) * 0.125
    *temp = (int16_t)temperature_binary * 0.125; 
    
  }
  else{
    // if its zero
    temperature_binary = temperature_binary >> 5;
    // since positive, C = Tempdata * 0.125C
    *temp = temperature_binary * 0.125;
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
