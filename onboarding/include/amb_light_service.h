#ifndef ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_
#define ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_

<<<<<<< HEAD
#include <stdint.h>
=======
#include "obc_errors.h"

>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d
/* Event types to be sent to light service queue */
typedef enum {
    MEASURE_LIGHT,
} light_event_t;

<<<<<<< HEAD
/* Light service task config */
#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256
#define LIGHT_SERVICE_PRIORITY 1

/* USER CODE BEGIN */
#define LIGHT_SERVICE_SIZE 10
#define LIGHT_QUEUE_SIZE sizeof(light_event_t)
#define ERROR_MESSAGE "Error" 
#define TEXT_SIZE 8

/* USER CODE END */

=======
>>>>>>> e95293f2e8ddbf374c16667cc3619425316cd73d
/**
 * @brief Create the light service task and queue.
 * 
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t initLightService(void);

/**
 * @brief Send an event to the light service queue.
 * 
 * @param event The event to send to the queue.
 * @note This function is used by the controller task to send events to the light service queue.
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t sendToLightServiceQueue(light_event_t *event);

#endif /* ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_ */
