#ifndef ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_
#define ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_

#include "obc_errors.h"
#include <stdint.h>

/* Event types to be sent to light service queue */
typedef enum {
    MEASURE_LIGHT,
} light_event_t;

/**
 * @brief Create the light service task and queue.
 * 
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t initLightService(void);

/**
 * @brief get data from ADC
 *
 * @param void
 * @note use in lightServiceTask(void *pvParameters)
 * @return uint32, return data value from sensor
 */
uint16_t getLightSensorData(void);

/**
 * @brief Send an event to the light service queue.
 * 
 * @param event The event to send to the queue.
 * @note This function is used by the controller task to send events to the light service queue.
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, error code otherwise
 */
obc_error_code_t sendToLightServiceQueue(light_event_t *event);

#endif /* ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_ */
