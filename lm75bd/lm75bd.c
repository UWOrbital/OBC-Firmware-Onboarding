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

#define CONF_WRITE_BUFF_SIZE 1U //should need to be a maximum of 2 as for setup we write dev addr then ptr byte, rest of the time, just write dev addr and get back 2 BYTES
#define CONF_READ_BUFF_SIZE 2U
#define LM75BD_PTR_BYTE 0x00U //should just be 0 as we want to read from the temp register. Thus this technically is not neccisary
#define LM75BD_PTR_SEND 2U
#define LM75BD_TEMP_READ 1U
error_code_t readTempLM75BD(uint8_t devAddr, float *temp) {
  /* Implement this driver function */
  
  /*Plan
  -As per fig 11, we need to write the device addr then the poiner byte to indicate we are reading temp
  -need to throw away first temp reading
  -
  
  
  */

  uint8_t WriteBuff[CONF_READ_BUFF_SIZE] = {0};//This buffer is for sending to LM75BD
  uint8_t ReadBuff[CONF_WRITE_BUFF_SIZE] = {0}; //for reading from LM75BD, should hold 2 bytes 


  ReadBuff[1] = LM75BD_PTR_BYTE;//Just 0 as we want to read


  errCode = i2cSendTo(LM75BD_OBC_I2C_ADDR, WriteBuff, CONF_WRITE_BUFF_SIZE);//Send the pointer byte, the address should already be sent by the function itsself
  if(errCode != ERR_CODE_SUCCESS) return errCode;// if we failed

  //ack and restart should happen after this.

  errCode = i2cReceiveFrom(LM75BD_OBC_I2C_ADDR, ReadBuff, LM75BD_TEMP_READ)//Recieve the temp data (this will be backwards as the device send MSB first)
  if(errCode != ERR_CODE_SUCCESS) return errCode;// if we failed


  //need to perform conversion on read in data(7.4.3), also msb in in index 0 and lsb is in index 1

  uint16_t temp = ReadBuff[1] | (ReadBuff[0] << 8);

  if((temp & (1 << 15)) == 1){//shift 1 left 15 bits to test d10 if it's a 1
    //negative temp, take twos compliment
    ~temp;//invert bits
    temp+1;//add 1
  }

  temp = (temp*125)/10;//need to retain numerical accuracy here.
  

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
