#pragma once

#include "lm75bd.h"
#include "errors.h"

// enum: group integer constants and name them
typedef enum {
  THERMAL_MGR_EVENT_MEASURE_TEMP_CMD,
  THERMAL_MGR_EVENT_OS_INTERRUPT,
} thermal_mgr_event_type_t;

// struct: can store different types, group multiple values
// can put an enum inside a struct
typedef struct {
  thermal_mgr_event_type_t type;
} thermal_mgr_event_t;

typedef struct {
  uint8_t devAddr;
  float overTempThresholdCelsius;
  float hysteresisThresholdCelsius;
} thermal_mgr_config_t;

#ifdef __cplusplus
extern "C" {
#endif

void initThermalSystemManager(lm75bd_config_t *config);

error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event);

void addTemperatureTelemetry(float tempC);

void overTemperatureDetected(void);

void safeOperatingConditions(void);

#ifdef __cplusplus
}
#endif
