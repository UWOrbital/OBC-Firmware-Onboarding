#ifndef ONBOARDING_INCLUDE_SERIAL_IO_H_
#define ONBOARDING_INCLUDE_SERIAL_IO_H_

#include <stdint.h>
#include <sci.h>

/**
 * @brief Initialize mutexes protecting SCI and SCI2.
 */
void sciMutexInit(void);

/**
 * @brief Send a string of text via SCI or SCI2.
 * @param sci The SCI register to use (sciREG or scilinREG).
 * @param text The text to send.
 * @param length The length of the text to send.
 * @return 1 if the text was sent, 0 otherwise.
 */
uint8_t sciPrintText(sciBASE_t *sci, unsigned char *text, uint32_t length);

#endif /* ONBOARDING_INCLUDE_SERIAL_IO_H_ */