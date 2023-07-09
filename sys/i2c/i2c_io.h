#pragma once

#include "errors.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void initI2C(void);

error_code_t i2cSendTo(uint8_t sAddr, uint8_t *buf, uint16_t numBytes);

error_code_t i2cReceiveFrom(uint8_t sAddr, uint8_t *buf, uint16_t numBytes);

#ifdef __cplusplus
}
#endif
