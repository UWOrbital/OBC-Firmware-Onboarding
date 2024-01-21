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

/*
 * @brief Reads temperature from LM75BD
 *
 * @param devAddr - Address of the device to communicate to  
 * @param temp - Float to store temperature in
 *
 */
error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {  
    float result = 0.0;
    uint8_t targetRegister = LM75BD_POINTER_TEMP;
    error_code_t errCode;
    // Select sensors internal temperature register using pointer register    
    LOG_IF_ERROR_CODE(i2cSendTo(devAddr, &targetRegister, sizeof(targetRegister)));
    // XXX: p.8 of datasheet suggests first temperature read is always incorrect
    
    // Perform an I2C read of the device
    uint8_t readTempBuf[2] = {0};   
    LOG_IF_ERROR_CODE(i2cReceiveFrom(devAddr, readTempBuf, sizeof(readTempBuf)));  
    // MSB is top half of readTempBuf
    uint16_t tempData =  readTempBuf[0] << 3 |      
                        (readTempBuf[1] >> 5);    
    
    // Check for out-of-range error
    if (tempData > (pow(2, 11) - 1)) {
        // Somehow we got erronious I2C data
        return ERR_CODE_UNKNOWN;
    }
    /*  uint16_t representation
     *  ________________________________________________________________________________
     *  15 | 14 | 13 | 12 | 11 | 10 | 9 | 8 || B7 | B6 | B5 | B4 | B3 | B2 | B1 | B0 ||
     *  --------------------------------------------------------------------------------
     *  0    0    0    0    0    D10  D9  D8   D7   D6   D5   D4   D3   D2   D1   D0
     *  --------------------------------------------------------------------------------
     *                           ^ LM75BD_TEMP_RA_MSB_MASK
     *                            |-----------LM75BD_TEMP_RA_10B_MASK--------------|
     */
    // Two's Complement
    if ((tempData & LM75BD_TEMP_RA_MSB_MASK) > 0) {
        // Result is negative, invert it and mask off top 5 bits    
        result = -1.0 * (~tempData & LM75BD_TEMP_RA_10B_MASK) * LM75BD_TEMP_LSB_TO_C;   // Calculate degrees C
        result -= LM75BD_TEMP_LSB_TO_C;     // We lose a MSB due to 2s Complement
    } else {
        // Result is positive
        result = (tempData & LM75BD_TEMP_RA_10B_MASK) * LM75BD_TEMP_LSB_TO_C;   // Convert into float

    }  
    // Store result;
    *temp = result;

    return ERR_CODE_SUCCESS;
}

#define CONF_WRITE_BUFF_SIZE 2U
error_code_t writeConfigLM75BD( uint8_t devAddr, 
                                uint8_t osFaultQueueSize, 
                                uint8_t osPolarity,
                                uint8_t osOperationMode, 
                                uint8_t devOperationMode) {
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
