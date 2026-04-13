#pragma once

/* DO NOT MODIFY ANYTHING IN THIS FILE */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize the mutex for the console.
 */
void initConsole(void);

/*
 * Print a string to the console.
 */
void printConsole(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
