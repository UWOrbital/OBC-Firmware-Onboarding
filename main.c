/* Include Files */
#include sys_common.h
#include "gio.h"

/* FreeRTOS Kernel includes. */
#include "FreeRTOS.h"
#include "os_portmacro.h"
#include "os_task.h"

#define BLINKY_STACK_SIZE 1024

#define BLINK1_NAME "Blink1"
#define BLINK1_PRIORITY 5
#define BLINK1_DELAY_TICKS 800

#define BLINK2_NAME "Blink2"
#define BLINK2_PRIORITY 5
#define BLINK2_DELAY_TICKS 200

xTaskHandle xTask1Handle;
xTaskHandle xTask2Handle;

void main(void)
{
    gioInit();
    BaseType_t retval1;
    BaseType_t retval2;

    retval1 = xTaskCreate(blinkGIOB1, BLINK1_NAME, BLINKY_STACK_SIZE, NULL, BLINK1_PRIORITY, &xTask1Handle);
    retval2 = xTaskCreate(blinkGIOB2, BLINK2_NAME, BLINKY_STACK_SIZE, NULL, BLINK2_PRIORITY, &xTask2Handle);

    if(retval1 == pdPASS && retval2 == pdPASS) {
        vTaskStartScheduler();
    }
}

void blinkGIOB1(void * arg)
{
    for(;;) {
        // Toggle on and off at a set speed
        gioToggleBit(gioPORTB, 1);
        vTaskDelay(BLINK1_DELAY_TICKS);
    }
}

void blinkGIOB2(void * arg)
{
    // First blink on and off, two cycles per one cycle on the other LED
    // Then blink on and off quickly -- four cycles per one cycle on the other LED
    for(;;) {
        for(int i = 0; i < 2; i++) {
            gioSetBit(gioPORTB, 2, TRUE);
            vTaskDelay(BLINK2_DELAY_TICKS * 2);
            gioSetBit(gioPORTB, 2, FALSE);
            vTaskDelay(BLINK2_DELAY_TICKS * 2);
        }

        for(int i = 0; i < 4; i++) {
            gioSetBit(gioPORTB, 2, TRUE);
            vTaskDelay(BLINK2_DELAY_TICKS);
            gioSetBit(gioPORTB, 2, FALSE);
            vTaskDelay(BLINK2_DELAY_TICKS);
        }
    }
}
