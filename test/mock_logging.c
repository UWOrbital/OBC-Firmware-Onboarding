#include "logging.h"
#include "errors.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

void logSetLevel(log_level_t newLogLevel) { logLevel = newLogLevel; }

error_code_t logLog(log_level_t msgLevel, const char *file, uint32_t line, const char *s, ...) {
  return ERR_CODE_SUCCESS;
}