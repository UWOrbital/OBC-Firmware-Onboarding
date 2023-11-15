#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"
#include "logging.h"

#include <stdint.h>
#include <string.h>
#include <math.h>

/* LM75BD Registers (p.8) */
#define LM75BD_REG_CONF 0x01U  /* Configuration Register (R/W) */
#define LM75BD_REG_TEMP 0x00U  /* Temperature Register (RO) */

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

// 2 Bytes of Data
#define CONF_POINTER_BUFF_SIZE 1U
#define CONF_READ_BUFF_SIZE 2U
#define MASK_SIGN_BIT 0x7FU
#define MASK_NINE_BIT 0x1FFU
/**
 * @brief Reads LM75BD temperature using I2C
 */
error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  /* Implement this driver function */
  error_code_t errCode;

  /* Write pointer register */
  uint8_t point_buff[CONF_POINTER_BUFF_SIZE] = {0};
  point_buff[0] = 0x00U;
 
  errCode = i2cSendTo(LM75BD_OBC_I2C_ADDR, point_buff, CONF_POINTER_BUFF_SIZE);
  if (errCode != ERR_CODE_SUCCESS)
    return errCode;

  /* Read temperature register */
  uint8_t read_buff[CONF_READ_BUFF_SIZE] = {0};
  errCode = i2cReceiveFrom(LM75BD_OBC_I2C_ADDR, read_buff, CONF_READ_BUFF_SIZE);
  if (errCode != ERR_CODE_SUCCESS)
    return errCode;

  // Concatenate D9 through D0; read data sheet sect 7.4.3
  // <https://www.nxp.com/docs/en/data-sheet/LM75B.pdf>
  uint16_t temp_data = ((read_buff[0] & MASK_SIGN_BIT) << 3) | (read_buff[1] >> 5);
  if ((read_buff[0] >> 7) & 0x01) {
    // D10 1; temperature negative .. convert from two's complement
    (*temp) = ((float) (~temp_data & MASK_NINE_BIT)+1) * -0.125;
  } else {
    (*temp) = ((float) temp_data) * 0.125; 
  }
    // D10 0; temperature positive

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
