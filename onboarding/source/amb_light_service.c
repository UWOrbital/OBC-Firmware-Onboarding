#include "amb_light_service.h"
#include "serial_io.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here

/* USER CODE END */

/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

uint8_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here.

    /* USER CODE END */
    return 1;
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.

    /* USER CODE END */
}

uint8_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue.
    
    /* USER CODE END */
    return 0;
}
