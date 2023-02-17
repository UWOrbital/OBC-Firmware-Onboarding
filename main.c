#include "serial_io.h"
#include "controller.h"
#include "obc_errors.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <adc.h>
#include <gio.h>
#include <sci.h>
#include <sys_common.h>

int main(void)
{
    /* Initialize hardware modules */
    gioInit();
    sciInit();
    adcInit();                 
    
    /* Initialize mutex that protects sci module */
    sciMutexInit();
    
    /* Create controller task and related timers */
    if (initController() != OBC_ERR_CODE_SUCCESS) {
        sciPrintf("Failed to create controller task and timers\r\n");
        while (1);
    }
    
    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();
}

