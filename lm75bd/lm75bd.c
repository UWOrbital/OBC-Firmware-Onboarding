#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U /* Configuration Register (R/W) */

// Read Write Constants
#define READ_BYTES 2
#define WRITE_BYTES 1

error_code_t lm75bdInit(lm75bd_config_t *config)
{
  error_code_t errCode;

  if (config == NULL)
    return ERR_CODE_INVALID_ARG;

  errCode = writeConfigLM75BD(config->devAddr, config->osFaultQueueSize, config->osPolarity,
                              config->osOperationMode, config->devOperationMode);

  if (errCode != ERR_CODE_SUCCESS)
    return errCode;

  // Assume that the overtemperature and hysteresis thresholds are already set
  // Hysteresis: 75 degrees Celsius
  // Overtemperature: 80 degrees Celsius

  return ERR_CODE_SUCCESS;
}

error_code_t readTempLM75BD(uint8_t devAddr, float *temp)
{
  error_code_t i2cFunctions; // Stores the return value from the i2cSendTo() and i2cReceiveFrom() functions

  if (temp == NULL)
  {
    return ERR_CODE_INVALID_ARG;
  }

  uint8_t accessBuffer = TEMP_REGISTER;

  i2cFunctions = i2cSendTo(devAddr, &accessBuffer, WRITE_BYTES);
  if (i2cFunctions != ERR_CODE_SUCCESS)
  {
    return i2cFunctions;
  }

  uint8_t readData[READ_BYTES];
  i2cFunctions = i2cReceiveFrom(devAddr, readData, READ_BYTES);
  if (i2cFunctions != ERR_CODE_SUCCESS)
  {
    return i2cFunctions;
  }

  int16_t processedTemp = ((uint16_t)readData[0] << 3) | ((uint16_t)(readData[1]) >> 5);

  if (readData[0] >> 7 == 0)
  { // positive temp
    *temp = processedTemp * 0.125;
  }
  else
  { // negative temp
    processedTemp = ~processedTemp + 1;
    // Negate everything, good, except now have 1111 at the front which i don't want
    // Use bitwise or to remove those
    processedTemp ^= 0xF800;
    *temp = -processedTemp * 0.125;
    // two's complement
    //- that * 0.125
  }

  return ERR_CODE_SUCCESS;
}

#define CONF_WRITE_BUFF_SIZE 2U
error_code_t writeConfigLM75BD(uint8_t devAddr, uint8_t osFaultQueueSize, uint8_t osPolarity,
                               uint8_t osOperationMode, uint8_t devOperationMode)
{
  error_code_t errCode;

  // Stores the register address and data to be written
  // 0: Register address
  // 1: Data
  uint8_t buff[CONF_WRITE_BUFF_SIZE] = {0};

  buff[0] = LM75BD_REG_CONF;

  uint8_t osFaltQueueRegData = 0;
  switch (osFaultQueueSize)
  {
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
  if (errCode != ERR_CODE_SUCCESS)
    return errCode;

  return ERR_CODE_SUCCESS;
}
