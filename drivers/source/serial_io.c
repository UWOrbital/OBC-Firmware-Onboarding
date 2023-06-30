#include "serial_io.h"
#include "obc_errors.h"
#include "obc_assert.h"

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

/* USER CODE BEGIN */
// Add a static assertion to ensure that UART_PRINT_REG is defined as either scilinREG or sciREG
STATIC_ASSERT(UART_PRINT_REG == sciREG || UART_PRINT_REG == scilinREG, "UART_PRINT_REG must be sciREG or scilinREG");
/* USER CODE END */
STATIC_ASSERT(MAX_PRINTF_SIZE > 0, "MAX_PRINTF_SIZE must be greater than 0");

/* GLOBAL VARIABLES */
static SemaphoreHandle_t sciMutex; // Protects SCI1/SCI module
static StaticSemaphore_t sciMutexBuffer; // Buffer for SCI mutex

static SemaphoreHandle_t sciLinMutex; // Protects SCI2/SCILin module
static StaticSemaphore_t sciLinMutexBuffer; // Buffer for SCI2 mutex

/**
 * @brief Iterate through an array of bytes and transmit them via UART_PRINT_REG.
 * 
 * @param bytes The array of bytes to transmit.
 * @param length The length of the array of bytes to transmit.
 * @return OBC_ERR_CODE_SUCCESS on success, else an error code
 */
static obc_error_code_t sciSendBytes(unsigned char *bytes, uint32_t length);

void sciMutexInit(void) {
    if (sciMutex == NULL) {
        sciMutex = xSemaphoreCreateMutexStatic(&sciMutexBuffer);
    }
    
    /* USER CODE BEGIN */
    // Create mutex to protect SCI2/SCILin module here.
    if (sciLinMutex == NULL){
        sciLinMutex = xSemaphoreCreateMutexStatic(&sciLinMutexBuffer);
    }
    /* USER CODE END */

    // Static allocation of mutexes can only fail if the buffer is NULL
    configASSERT(sciMutex != NULL);
    configASSERT(sciLinMutex != NULL);
}

obc_error_code_t sciPrintText(unsigned char *text, uint32_t length) {
    if (text == NULL || length == 0)
        return OBC_ERR_CODE_INVALID_ARG;

    SemaphoreHandle_t mutex = (UART_PRINT_REG == sciREG) ? sciMutex : sciLinMutex;
    if (mutex == NULL)
        return OBC_ERR_CODE_SCI_INIT_NOT_CALLED;

    /* USER CODE BEGIN */
    // Print text to the serial port using sciSendBytes. Use the mutex to protect the SCI module.
    // take mutex
    if (sciLinMutex != NULL){
        if (xSemaphoreTake( sciLinMutex, ( TickType_t ) UART_MUTEX_BLOCK_TIME == pdTRUE)){
            
            obc_error_code_t serialOutput = sciSendBytes(text, length);
            xSemaphoreGive(sciLinMutex);

            return serialOutput;
        }

        return OBC_ERR_CODE_MUTEX_TIMEOUT;
    }
    // execute code
    // lock mutex
    /* USER CODE END */
}

obc_error_code_t sciPrintf(const char *s, ...){
    if (s == NULL)
        return OBC_ERR_CODE_INVALID_ARG;

    char buf[MAX_PRINTF_SIZE] = {0};
    
    va_list args;
    va_start(args, s);
    int n = vsnprintf(buf, MAX_PRINTF_SIZE, s, args);
    va_end(args);

    if (n < 0)
        return OBC_ERR_CODE_INVALID_ARG;

    // n == MAX_PRINTF_SIZE invalid because null character isn't included in count
    if ((uint32_t)n >= MAX_PRINTF_SIZE)
        return OBC_ERR_CODE_INVALID_ARG;

    return sciPrintText((unsigned char *)buf, MAX_PRINTF_SIZE);
}

static obc_error_code_t sciSendBytes(unsigned char *bytes, uint32_t length) {
    if (bytes == NULL || length == 0)
        return OBC_ERR_CODE_INVALID_ARG;
    
    for (uint32_t i = 0; i < length; i++) {
        if (bytes[i] == '\0')
            break;
            
        // sciSendByte waits for the transmit buffer to be empty before sending
        sciSendByte(UART_PRINT_REG, bytes[i]);
    }
    
    return OBC_ERR_CODE_SUCCESS;
}
