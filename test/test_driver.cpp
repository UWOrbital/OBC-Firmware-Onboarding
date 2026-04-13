#include <gtest/gtest.h>
#include <fff.h>

#include "lm75bd.h"
#include "i2c_io.h"
#include "errors.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(error_code_t, i2cSendTo, uint8_t, uint8_t *, uint16_t);
FAKE_VALUE_FUNC(error_code_t, i2cReceiveFrom, uint8_t, uint8_t *, uint16_t);

#define FFF_FAKES_LIST(FAKE)  \
  FAKE(i2cSendTo)             \
  FAKE(i2cReceiveFrom)


void resetFakes(void) {
  /* Register resets */
  FFF_FAKES_LIST(RESET_FAKE);

  /* reset common FFF internal structures */
  FFF_RESET_HISTORY();
}

uint8_t customFakeTxBuff[2] = {0};

// This function is needed because the buff passed into i2cSendTo goes out of scope
// before the test can check it
error_code_t i2cSendTo_custom_fake(uint8_t addr, uint8_t *buff, uint16_t numBytes) {
  if (addr != 0x4FU) {
    return ERR_CODE_I2C_TRANSFER_TIMEOUT;
  }

  uint8_t numToCopy = numBytes < 2 ? numBytes : 2;
  memcpy(customFakeTxBuff, buff, numToCopy);

  return ERR_CODE_SUCCESS;
}

TEST(TestLm75bdDriver, TestWriteConfigLm75bdSuccess) {
  resetFakes();

  uint8_t addr = 0x4FU;

  i2cSendTo_fake.custom_fake = i2cSendTo_custom_fake;
  i2cSendTo_fake.return_val = ERR_CODE_SUCCESS;

  ASSERT_EQ(writeConfigLM75BD(0x4FU, 1, 0, 1, 0), ERR_CODE_SUCCESS);

  const uint8_t expectedBuff[2] = {0x1U, 0b00000010U};

  EXPECT_EQ(i2cSendTo_fake.call_count, 1);
  EXPECT_EQ(i2cSendTo_fake.arg0_val, addr); // Check device address
  EXPECT_EQ(memcmp(customFakeTxBuff, expectedBuff, 2), 0); // Check buffer (reg addr + config byte)  
  EXPECT_EQ(i2cSendTo_fake.arg2_val, 2U); // Check numBytes
}


static uint8_t customTempBuff[2] = {0};
error_code_t getTemp_custom_fake(uint8_t addr, uint8_t *buff, uint16_t numBytes) {
  if (addr != 0x4FU) {
    return ERR_CODE_I2C_TRANSFER_TIMEOUT;
  }

  uint8_t numToCopy = numBytes < 2 ? numBytes : 2;
  memcpy(buff, customTempBuff, numToCopy);

  return ERR_CODE_SUCCESS;
}

TEST(TestLm75bdDriver, TestReadTempLm75bdPosTempSuccess) {
  resetFakes();

  uint8_t addr = 0x4FU;
  float temp;

  // 127 deg C -> 01111111 000XXXXX
  customTempBuff[0] = 0x7FU;
  customTempBuff[1] = 0x00U;

  i2cReceiveFrom_fake.custom_fake = getTemp_custom_fake;

  ASSERT_EQ(readTempLM75BD(addr, &temp), ERR_CODE_SUCCESS);
  EXPECT_EQ(temp, 127.0f);
}

TEST(TestLm75bdDriver, TestReadTempLm75bdNegTempSuccess) {
  resetFakes();

  uint8_t addr = 0x4FU;
  float temp;

  // -25 deg C -> 11100111 000XXXXX
  customTempBuff[0] = 0xE7;
  customTempBuff[1] = 0x00U;

  i2cReceiveFrom_fake.custom_fake = getTemp_custom_fake;

  ASSERT_EQ(readTempLM75BD(addr, &temp), ERR_CODE_SUCCESS);
  EXPECT_EQ(temp, -25.0f);
}

TEST(TestLm75bdDriver, TestReadTempLm75bdCallSequence) {
  resetFakes();

  uint8_t addr = 0x4FU;
  float temp;

  i2cSendTo_fake.custom_fake = i2cSendTo_custom_fake;
  i2cReceiveFrom_fake.return_val = ERR_CODE_SUCCESS;

  ASSERT_EQ(readTempLM75BD(addr, &temp), ERR_CODE_SUCCESS);

  EXPECT_EQ(i2cReceiveFrom_fake.call_count, 1);
  EXPECT_EQ(i2cSendTo_fake.call_count, 1);

  // Check call sequence
  EXPECT_EQ(fff.call_history[0], (void *)i2cSendTo);
  EXPECT_EQ(fff.call_history[1], (void *)i2cReceiveFrom);

  // Check call arguments
  EXPECT_EQ(i2cSendTo_fake.arg0_val, addr); // Check device address
  EXPECT_EQ(customFakeTxBuff[0], 0x0U); // Check buffer (reg addr)
  EXPECT_EQ(i2cSendTo_fake.arg2_val, 1U); // Check numBytes

  EXPECT_EQ(i2cReceiveFrom_fake.arg0_val, addr); // Check device address
  EXPECT_EQ(i2cReceiveFrom_fake.arg2_val, 2U); // Check numBytes
}
