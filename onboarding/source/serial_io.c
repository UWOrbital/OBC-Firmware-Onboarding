#include "serial_io.h"

#include "FreeRTOS.h"
#include "os_portmacro.h"
#include "os_semphr.h"

static SemaphoreHandle_t sciCommMutex = NULL;

void sciMutexInit(void) {
    if (sciCommMutex == NULL) {
        sciCommMutex = xSemaphoreCreateMutex();
    }
}

uint8_t sciPrintText(sciBASE_t *sci, unsigned char *text, uint32_t length) {
    if (sciCommMutex != NULL) {
        if (xSemaphoreTake(sciCommMutex, portMAX_DELAY) == pdTRUE) {
            while (length--) {
                while ((sci->FLR & 0x4) == 4);
                sciSendByte(sci, *text++);
            }
            xSemaphoreGive(sciCommMutex);
            return 1;
        }
    }
    return 0;
}