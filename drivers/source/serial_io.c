#include "serial_io.h"

#include <FreeRTOS.h>
#include <os_portmacro.h>
#include <os_semphr.h>

#include <sci.h>

static SemaphoreHandle_t sciMutex = NULL; // Protects SCI
static SemaphoreHandle_t sciLinMutex = NULL; // Protects SCI2

/**
 * @brief Iterate through an array of bytes and transmit them via SCI or SCI2.
 * 
 * @param sci The SCI register to use (sciREG or scilinREG).
 * @param bytes The array of bytes to transmit.
 * @param length The length of the array of bytes to transmit.
 */
static void sciSendBytes(sciBASE_t *sci, unsigned char *bytes, uint32_t length);

void sciMutexInit(void) {
    if (sciMutex == NULL) {
        sciMutex = xSemaphoreCreateMutex();
    }
    /* USER CODE BEGIN */
    // Create mutex to protect SCI2/SCILin module here.
    if (sciLinMutex == NULL) {
        sciLinMutex = xSemaphoreCreateMutex();
    }
    /* USER CODE END */
}

uint8_t sciPrintText(sciBASE_t *sci, unsigned char *text, uint32_t length) {
    /* initSciMutex must be called before printing is allowed */
    sciMutexInit();
    ASSERT(sciMutex != NULL);

    if (sci == sciREG) {
        if (sciMutex != NULL) {
            if (xSemaphoreTake(sciMutex, portMAX_DELAY) == pdTRUE) {
                sciSendBytes(sci, text, length);
                xSemaphoreGive(sciMutex);
                return 1;
            }
        }
    }
    /* USER CODE BEGIN */
    // Print text to the SCILin serial port here.
    ASSERT(sciLinMutex != NULL);

    if (sci == scilinREG) {
        if (sciLinMutex != NULL) {
            if (xSemaphoreTake(sciLinMutex, portMAX_DELAY) == pdTRUE) {
                sciSendBytes(sci, text, length);
                xSemaphoreGive(sciLinMutex);
                return 1;
            }
        }
    }
    /* USER CODE END */
    
    return 0;
}

static void sciSendBytes(sciBASE_t *sci, unsigned char *bytes, uint32_t length) {
    /* Note that an unsigned char is the same thing as a uint8_t */
    for (int i = 0; i < length; i++) {
        // sciSendByte waits for the transmit buffer to be empty before sending
        sciSendByte(sci, bytes[i]);
    }
} 