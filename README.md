# OnboardingDebug - Level 1

There are 5 errors in main.c which will prevent it from running. Can you find them all?

The intended behaviour of the code is to control to LEDs on the devboard - the blink pattern should be:
<pre>
LED A: ||: o       |x       |o       |x       :||     
LED B: ||: o   x   |o   x   |o x o x |o x o x :||
</pre>
...Where O represents an LED turning on, and X represents an LED turning off. Each LED is controlled in its own task.

The errors are mostly classic C programming mistakes. If you don't have a background in operating systems, or freeRTOS specifically, you should still be able to find all the bugs with C knowledge and basic pattern recognition skills. Good luck!
