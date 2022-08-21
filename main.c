#include "serial_io.h"
#include "controller.h"

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
    initController();
    
    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();
}
