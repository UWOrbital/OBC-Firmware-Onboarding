#ifndef ONBOARDING_INCLUDE_SERIAL_IO_H_
#define ONBOARDING_INCLUDE_SERIAL_IO_H_

#include <stdint.h>
#include "sci.h"

/**
 * @brief Initialize the mutex that prevents concurrent access to the SCI interface.
 */
void sciMutexInit(void);

/**
 * @brief Print text via the SCI port.
 * 
 * @param sci The sci register. Use scilinREG for usb-uart communication on LaunchPad.
 * @param text The text to print
 * @param length The length of the text
 * 
 * @note  The receiving device must have the correct serial port settings.
 * @return 1 if the text was printed, 0 if not.
 */
uint8_t sciPrintText(sciBASE_t *sci, unsigned char *text, uint32_t length);

#endif /* ONBOARDING_INCLUDE_SERIAL_IO_H_ */