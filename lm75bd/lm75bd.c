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

/**
 * @brief Reads current temperature from the temperature sensor
 *
 * @param devAddr The address of the device to read from.
 * @param temp sensor internal register
 */
error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {

    // transmit to sensor
    uint8_t buff_send = 0x00;
    uint8_t num_bytes = 1;

    i2cSendTo(devAddr, &buff_send, num_bytes);

    // recieve from sensor
    uint8_t buff_rec[2] = {0};
    num_bytes = 2; // from fig. 10

   i2cReceiveFrom(devAddr, buff_rec, num_bytes);

   // get the temperature in degrees Celsius
    /*
        |BS7|BS6|BS5|BS4|BS3|BS2|BS1|BS0|BR7|BR6|BR5|
        |D10|D 9|D 8|D 7|D 6|D 5|D 4|D 3|D 2|D 1|D 0|
        we dropped the unused bits in BR
   */
   uint16_t tempd = (buff_rec[0] << 3) | (buff_rec[1] >> 5);

    if ((buff_rec[0] & 0x80) == 0) { // D10 == 0
        *temp = tempd * 0.125f;
   } else {
        // 2's complement
        uint16_t comp = (~tempd & 0x07FF) + 1; // 0x07FF to get  11 bits

        *temp = comp * -0.125f;
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

  errCode = i2cSendTo(devAddr, buff, CONF_WRITE_BUFF_SIZE);
  if (errCode != ERR_CODE_SUCCESS) return errCode;

  return ERR_CODE_SUCCESS;
}
