#pragma once

#include "errors.h"

#include <stdint.h>
#include <stdarg.h>

/**
 * @enum log_level_t
 * @brief Log levels enum.
 *
 * Enum containing all log levels.
 */
typedef enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL, LOG_OFF } log_level_t;

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_TRACE
#endif

#define LOG_TRACE(...) logLog(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) logLog(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) logLog(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) logLog(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) logLog(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) logLog(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ERROR_CODE(errCode) LOG_ERROR("Error code: %lu", (uint32_t)errCode)

#define RETURN_IF_ERROR_CODE(_ret)         \
  do {                                     \
    errCode = _ret;                        \
    if (errCode != ERR_CODE_SUCCESS) { \
      LOG_ERROR_CODE(errCode);             \
      return errCode;                      \
    }                                      \
  } while (0)

#define LOG_IF_ERROR_CODE(_ret)            \
  do {                                     \
    errCode = _ret;                        \
    if (errCode != ERR_CODE_SUCCESS) { \
      LOG_ERROR_CODE(errCode);             \
    }                                      \
  } while (0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the logger
 */
void initLogger(void);

/**
 * @brief Set the logging level
 *
 * @param newLogLevel The new logging level
 */
void logSetLevel(log_level_t newLogLevel);

/**
 * @brief Log a message
 *
 * @param msgLevel				Level of the message
 * @param file					File of message
 * @param line					Line of message
 * @param s						Message to log
 * @param ...					Additional arguments for the message
 * @return error_code_t
 *
 */
error_code_t logLog(log_level_t msgLevel, const char *file, uint32_t line, const char *s, ...);

#ifdef __cplusplus
}
#endif
