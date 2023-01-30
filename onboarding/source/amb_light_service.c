#include "amb_light_service.h"
#include "serial_io.h"
#include "obc_errors.h"

#include <adc.h>
#include <sci.h>

/* USER CODE BEGIN */
// Include any additional headers and global variables here

/* USER CODE END */

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256
#define LIGHT_SERVICE_PRIORITY 1

/* USER CODE BEGIN */
// Define light service queue config here

/* USER CODE END */


/**
 * @brief Task that waits for MEASURE_LIGHT event and then prints the ambient light value to the serial port.
 * @param pvParameters Task parameters
 */
static void lightServiceTask(void * pvParameters);

obc_error_code_t initLightService(void) {
    /* USER CODE BEGIN */
    // Create the task and queue here. Return error code if task/queue was not created successfully.

    /* USER CODE END */
}

static void lightServiceTask(void * pvParameters) {
    /* USER CODE BEGIN */
    // Wait for MEASURE_LIGHT event in the queue and then print the ambient light value to the serial port.

    /* USER CODE END */
}

obc_error_code_t sendToLightServiceQueue(light_event_t *event) {
    /* USER CODE BEGIN */
    // Send the event to the queue. Return error code if event was not sent successfully.
    
    /* USER CODE END */
}
