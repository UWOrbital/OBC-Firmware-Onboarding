#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"
#include "logging.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_TEMP 0x00U /* Temperature register */
#define LM75BD_REG_CONF 0x01U /* Configuration Register (R/W) */

error_code_t lm75bdInit(lm75bd_config_t *config)
{
  error_code_t errCode;

  if (config == NULL)
    return ERR_CODE_INVALID_ARG;

  RETURN_IF_ERROR_CODE(writeConfigLM75BD(config->devAddr, config->osFaultQueueSize, config->osPolarity,
                                         config->osOperationMode, config->devOperationMode));

  // Assume that the overtemperature and hysteresis thresholds are already set
  // Hysteresis: 75 degrees Celsius
  // Overtemperature: 80 degrees Celsius

  return ERR_CODE_SUCCESS;
}

error_code_t readTempLM75BD(uint8_t devAddr, float *temp)
{
  if (temp == NULL)
    return ERR_CODE_INVALID_ARG;

  error_code_t errCode;

  uint8_t tempRegister = LM75BD_REG_TEMP;
  errCode = i2cSendTo(devAddr, &tempRegister, 1);

  if (errCode != ERR_CODE_SUCCESS)
    return errCode;

  uint8_t buffer[2] = {0};
  errCode = i2cReceiveFrom(devAddr, buffer, 2);

  if (errCode != ERR_CODE_QUEUE_FULL)
    return errCode;

  // Combines the 2 bytes into one 16 bit integer
  // and drops the 5 least siginificant bits
  int16_t tempData = ((buffer[0] << 8) | buffer[1]) >> 5;

  *temp = (float)(tempData) * 0.125f;

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
