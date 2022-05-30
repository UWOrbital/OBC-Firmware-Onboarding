# OnboardingDebug - Level 1

There are 5 errors in main.c which will prevent it from running. Can you find them all?

The intended behaviour of the code is to control to LEDs on the devboard - the blink pattern should be:
<pre>
LED A: ||: o       |x       |o       |x       :||     
LED B: ||: o   x   |o   x   |o x o x |o x o x :||
</pre>
...Where O represents an LED turning on, and X represents an LED turning off. Each LED is controlled in its own task.

The errors are mostly classic C programming mistakes. If you don't have a background in operating systems, or freeRTOS specifically, you should still be able to find all the bugs with C knowledge and basic pattern recognition skills. Good luck!

## Bugs Found:
* add quotes to include on line 2: `#include sys_common.h` -> `#include "sys_common.h"`
* add second task handle on line 20: `xTaskHandle xTask1Handle;` -> `xTaskHandle xTask1Handle, xTask2Handle;`
* add semicolon on line 24: `BaseType_t retval1, retval2` -> `BaseType_t retval1, retval2;`
* create second task after line 26: ` ` -> `retval2 = xTaskCreate( blinkGIOB2, BLINK2_NAME, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xTask2Handle );`
* adjust delay times on (line numbers are after fourth bug)
    * lines 55/57: `vTaskDelay( BLINK2_DELAY_TICKS );` -> `vTaskDelay( BLINK2_DELAY_TICKS * 2 );`
    * lines 62/64: `vTaskDelay( BLINK2_DELAY_TICKS * 2 );` -> `vTaskDelay( BLINK2_DELAY_TICKS );`