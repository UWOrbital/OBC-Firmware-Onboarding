#include "serial_io.h"
#include "obc_errors.h"

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <os_portmacro.h>
#include <os_semphr.h>
#include <os_task.h>

#include <sci.h>
#include <stdarg.h>
#include <stdio.h>

/* GLOBAL DEFINES */
#define UART_PRINT_REG scilinREG
#define MAX_PRINTF_SIZE 128U
#define UART_MUTEX_BLOCK_TIME portMAX_DELAY

/* GLOBAL VARIABLES */
static SemaphoreHandle_t sciMutex = NULL; // Protects SCI1/SCI module
static StaticSemaphore_t sciMutexBuffer; // Buffer for SCI mutex

static SemaphoreHandle_t sciLinMutex = NULL; // Protects SCI2/SCILin module
static StaticSemaphore_t sciLinMutexBuffer; // Buffer for SCI2 mutex

/**
 * @brief Iterate through an array of bytes and transmit them via UART_PRINT_REG.
 * 
 * @param bytes The array of bytes to transmit.
 * @param length The length of the array of bytes to transmit.
 */
static void sciSendBytes(unsigned char *bytes, uint32_t length);

void sciMutexInit(void) {
    if (sciMutex == NULL) {
        sciMutex = xSemaphoreCreateMutex();
    }
    /* USER CODE BEGIN */
    // Create mutex to protect SCI2/SCILin module here.
    
    /* USER CODE END */

    configASSERT(sciMutex != NULL);
    configASSERT(sciLinMutex != NULL);
}

obc_error_code_t sciPrintText(unsigned char *text, uint32_t length) {
    /* initSciMutex must be called before printing is allowed */
    configASSERT((UART_PRINT_REG == sciREG) || (UART_PRINT_REG == scilinREG));

    if (text == NULL || length == 0)
        return OBC_ERR_CODE_INVALID_ARG;

    SemaphoreHandle_t mutex = (UART_PRINT_REG == sciREG) ? sciMutex : sciLinMutex;
    configASSERT(mutex);

    /* USER CODE BEGIN */
    // Print text to the serial port using sciSendBytes.

    /* USER CODE END */
    
    return 0;
}

static void sciSendBytes(unsigned char *bytes, uint32_t length) {
    if (bytes == NULL || length == 0)
        return;
    
    for (int i = 0; i < length; i++) {
        if (bytes[i] == '\0')
            return;
            
        // sciSendByte waits for the transmit buffer to be empty before sending
        sciSendByte(UART_PRINT_REG, bytes[i]);
    }
}

obc_error_code_t sciPrintf(const char *s, ...){
    if (s == NULL)
        return OBC_ERR_CODE_INVALID_ARG;

    char buf[MAX_PRINTF_SIZE] = {0};
    
    va_list args;
    va_start(args, s);
    int n = vsnprintf(buf, MAX_PRINTF_SIZE, s, args);
    va_end(args);

    // n == MAX_PRINTF_SIZE invalid because null character isn't included in count
    if (n < 0 || n >= MAX_PRINTF_SIZE)
        return OBC_ERR_CODE_INVALID_ARG;

    return sciPrintText((unsigned char *)buf, MAX_PRINTF_SIZE);
}