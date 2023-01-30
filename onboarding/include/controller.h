#ifndef ONBOARDING_INCLUDE_CONTROLLER_H_
#define ONBOARDING_INCLUDE_CONTROLLER_H_

#include "obc_error.h"
#include <stdint.h>

/**
 * @brief Initialize the controller task and any timers.
 * 
 * @return obc_error_code_t OBC_ERR_CODE_SUCCESS if successful, otherwise an error code. 
 */
obc_error_code_t initController(void);

#endif /* ONBOARDING_INCLUDE_CONTROLLER_H_ */