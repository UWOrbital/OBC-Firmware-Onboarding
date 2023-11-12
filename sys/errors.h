
#pragma once

/* Define new error codes here if needed */

typedef enum {
    /* Common Errors 0 - 99 */
    ERR_CODE_SUCCESS = 0,
    ERR_CODE_UNKNOWN = 1,
    ERR_CODE_INVALID_ARG = 2,
    ERR_CODE_INVALID_STATE = 3,

    /* FreeRTOS errors */
    ERR_CODE_MUTEX_TIMEOUT = 100,
    ERR_CODE_QUEUE_FULL = 101,
    ERR_CODE_INVALID_QUEUE_MSG = 102,

    /* Driver errors */
    ERR_CODE_I2C_TRANSFER_TIMEOUT = 200,

    /* Logging errors */
    ERR_CODE_BUFF_TOO_SMALL = 300,
    ERR_CODE_LOG_MSG_SILENCED = 301,
} error_code_t;

