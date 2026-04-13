#pragma once

#include "errors.h"
#include <stdint.h>

/* DO NOT MODIFY ANYTHING IN THIS FILE */

#ifdef __cplusplus
extern "C" {
#endif

/* Driver functions. DO NOT MODIFY these function declarations.*/

/**
 * @brief Initializes the mutex used to protect the I2C bus.
 */
void initI2C(void);

/**
 * @brief Sends the contents of buf to the device with address sAddr.
 * 
 * @param sAddr The address of the device to send to.
 * @param buf The buffer containing the data to send.
 * @param numBytes The number of bytes to send.
 * @return error_code_t An error code indicating the status of the operation.
 * @note: Assume this function will handle transmitting the device address and setting
 * the R/W bit before sending data.
 */
error_code_t i2cSendTo(uint8_t sAddr, uint8_t *buf, uint16_t numBytes);

/**
 * @brief Receives numBytes from the device with address sAddr and stores them in buf.
 * 
 * @param sAddr The address of the device to receive from.
 * @param buf The buffer to store the received data in.
 * @param numBytes The number of bytes to receive.
 * @return error_code_t An error code indicating the status of the operation.
 * @note: Assume this function will handle transmitting the device address and setting 
 * the R/W bit before receiving data.
 */
error_code_t i2cReceiveFrom(uint8_t sAddr, uint8_t *buf, uint16_t numBytes);


/* Functions used to set up test environment */
/* DO NOT TOUCH */

void setLm75bdNextTempRegVal(uint16_t val);

uint16_t getLm75bdNextTempRegVal(void);

void setOsActive(uint8_t val);

uint8_t getOsActive(void);

#ifdef __cplusplus
}
#endif
