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

error_code_t readTempLM75BD(uint8_t devAddr, float *temp) { // Question 1. what do I use temp for, if theregister I am using is 0x00
  /* Implement this driver function */
    error_code_t errCode;

    if (temp == NULL) return ERR_CODE_INVALID_ARG;

    // to implement this we need to select the sensor's internaltemeprature register (7.4.1)
	  uint8_t pointer_register_value = 0x00; // Point to the Temperature Register
		devAddr = LM75BD_OBC_I2C_ADDR; 
		// once selected we can read temperature from the sensor 
		// errCode = i2cSendTo(LM75BD_OBC_I2C_ADDR, pointer_register_value, 2); 

    // error_code_t i2cSendTo(uint8_t sAddr, uint8_t *buf, uint16_t numBytes);

    // error_code_t i2cReceiveFrom(uint8_t sAddr, uint8_t *buf, uint16_t numBytes);

    
    // uint16_t numBytes = 2; 
    errCode = i2cSendTo(devAddr,&pointer_register_value, 1U); // read the temperature from the sensor
		if (errCode != ERR_CODE_SUCCESS) return errCode;
	
		uint8_t temp_raw[2]; // Temperature data is 16 bits (2 bytes) 
	  // You should have an I2C read function to receive data from the sensor
	  // errCode = i2cReceiveFrom(LM75BD_OBC_I2C_ADDR, temp_raw, 2);
    errCode = i2cReceiveFrom(devAddr, &temp_raw, 2);
		if (errCode != ERR_CODE_SUCCESS) return errCode;
			
		// // -- THIS SECTION IS NOT DONE (REVIEW 7.43)
		// // temp register = two 8-bit data bytes consisting of MSB and LSB 
		// // 11 bits are used to store  Temp data in two’s complement format with the resolution of 0.125 C
		// // D10 D9 D8 D7 D6 D5 D4 D3 | D2 D1 D0 X X X X X 
    
		uint16_t temp_data = (temp_raw[0] << 8) | temp_raw[1]; // we want to get D10 no? 

		// // Now the temperature data is stored in temp_data now convert to Celcius (7.4.3)
    // // Mask off irrelevant bits

    // if(temp_data & 0x0400){
    //     // temperature is positive 
    //     *temp = (temp_data >> 5) * 0.125; 
    // }else{
    //     // temperature is negative 
    //     temp_data = ~temp_data + 1; // Convert to positive representation
    //     int16_t temp_int = (int16_t)(temp_data >> 5);
    //     *temp = -((temp_int) >> 5) * 0.125; 
    // }

    const uint16_t sign_bit_mask = 0x0400;
    const uint16_t temperature_bits_mask = 0x03FF;

    // Extract the sign bit
    uint16_t sign_bit = temp_data & sign_bit_mask;

    if (sign_bit == 0) {
        // Temperature is positive
        *temp = (temp_data >> 5) * 0.125;
    } else {
        // Temperature is negative
        // Convert to positive representation using two's complement
        int16_t temp_int = (int16_t)temp_data;
        temp_int = ~temp_int + 1;
        *temp = -(temp_int >> 5) * 0.125;
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

//  Implement fcn readTempLM75BD() to read current temperatire from the temperature sensor 