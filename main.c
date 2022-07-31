#include "serial_io.h"
#include "controller.h"

#include <FreeRTOS.h>
#include <os_task.h>

#include <adc.h>
#include <sci.h>
#include <sys_common.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    /* initialize hardware modules */
    sciInit();
    adcInit();                 
    
    sciMutexInit();
    
    initController();
    
    vTaskStartScheduler();
}

