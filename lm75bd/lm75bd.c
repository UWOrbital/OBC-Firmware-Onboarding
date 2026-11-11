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

#define POINTER_BYTE_REG 0x00U
#define CELSIUS_CONVERSION 0.125f
error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  error_code_t errCode;

  if (temp == NULL) {
    return ERR_CODE_INVALID_ARG;
  }

  uint8_t tx_buff[1U] = { POINTER_BYTE_REG };
  // writing one byte to i2c reg, will recieve 2 bytes of temp val
  RETURN_IF_ERROR_CODE(i2cSendTo(devAddr, tx_buff, sizeof(tx_buff)));

  uint8_t rx_buff[2U];
  RETURN_IF_ERROR_CODE(i2cReceiveFrom(devAddr, rx_buff, sizeof(rx_buff)));

  uint16_t bits = (rx_buff[0] << 8) | rx_buff[1];
  // shifts temp to an 11 bit num: D10 at bit 10, and D0 at bit 0
  uint16_t temp_bits = bits >> 5;

  if ((temp_bits & (1 << 10)) == 0) {
    // postitive temp reading
    *temp = (float)temp_bits * CELSIUS_CONVERSION;
  } else {
    // twos compliment if negative num
    int16_t compliment = -((temp_bits ^ 0x07FF) + 1);
    *temp = (float)compliment * CELSIUS_CONVERSION;
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
