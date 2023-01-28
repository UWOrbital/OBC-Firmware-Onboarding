#ifndef ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_
#define ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_

#include <stdint.h>

/* Event types to be sent to light service queue */
typedef enum {
    NULL_EVENT,
    MEASURE_LIGHT
} light_event_t;

/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256
#define LIGHT_SERVICE_PRIORITY 1

/* USER CODE BEGIN */
// Define light service queue config here
#define LIGHT_EVENT_SIZE (sizeof (light_event_t))
#define QUEUE_SiZE 1
/* USER CODE END */

/**
 * @brief Create the light service task and queue.
 * 
 * @return uint8_t 1 if successful, 0 otherwise 
 */
uint8_t initLightService(void);

/**
 * @brief Send an event to the light service queue.
 * 
 * @param event The event to send to the queue.
 * @note This function is used by the controller task to send events to the light service queue.
 * @return uint8_t 1 if successful, 0 otherwise 
 */
uint8_t sendToLightServiceQueue(light_event_t *event);

#endif /* ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_ */
