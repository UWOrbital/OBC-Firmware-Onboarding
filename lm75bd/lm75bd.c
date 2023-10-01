#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */

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
  uint8_t bufRead[1];
  uint8_t bufWrite[2];
  bufRead[0] = 0;
  if (temp == NULL)
  {
    return ERR_CODE_INVALID_ARG;
  }
  error_code_t sendResult = i2cSendTo(devAddr, bufRead , 1);
  if (sendResult != ERR_CODE_SUCCESS)
  {
    return sendResult;
  }
  error_code_t receiveResult = i2cReceiveFrom(devAddr,bufWrite, 2);
  if (receiveResult != ERR_CODE_SUCCESS)
  {
    return receiveResult;
  }
  int16_t tempReading = 0;
  /* Sets the first 8 bits of tempReading to the MSB in index 0 of buf2 (which stored the temperature data from the sensor)
    then shifts the byte 8 to the left so it occupies the first 8 bits
  )*/
  tempReading |= ((bufWrite[0] << 8) | bufWrite[1]) >> 5;
  /* If 11th bit (MSB) is less than 1 (1024), then assigns temp the value of tempReading the formula for + temp reading
  Otherwise, inverts the last 11 bits of tempReading and finds the two's complement for the - temp reading.
  Divides all values by 8.0.
  */
  
  if (tempReading < 1024)
  {
    *temp = (float)tempReading * 0.125;
  }

  else
  {
    tempReading  = tempReading ^ 0b0000011111111111;
    *temp = (float)((-(tempReading + 1.0)) *0.125);
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
