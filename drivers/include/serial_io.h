#ifndef ONBOARDING_INCLUDE_SERIAL_IO_H_
#define ONBOARDING_INCLUDE_SERIAL_IO_H_

#include "obc_errors.h"

#include <stdint.h>

/**
 * @brief Initialize mutexes protecting SCI and SCI2.
 */
void sciMutexInit(void);

/**
 * @brief Send a string of text via UART_PRINT_REG.
 * 
 * @param text The text to send.
 * @param length The length of the text to send.
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t sciPrintText(unsigned char *text, uint32_t length);

/**
 * @brief Printf via UART_PRINT_REG.
 * 
 * @param s The format string
 * @param ... Arguments to use in format string
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
obc_error_code_t sciPrintf(const char *s, ...);

#endif /* ONBOARDING_INCLUDE_SERIAL_IO_H_ */