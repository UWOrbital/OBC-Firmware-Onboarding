#pragma once

#include "errors.h"

#include <stdint.h>

/* LM75BD I2C Device Address */
#define LM75BD_OBC_I2C_ADDR 0x4F

/* LM75BD Configuration Values */
#define LM75BD_DEV_OP_MODE_NORMAL 0x00U
#define LM75BD_DEV_OP_MODE_SHUTDOWN 0x01U

#define LM75BD_OS_POL_ACTIVE_LOW 0x00U
#define LM75BD_OS_POL_ACTIVE_HIGH 0x01U

#define LM75BD_OS_OP_MODE_COMP 0x00U
#define LM75BD_OS_OP_MODE_INT 0x01U

// Default temperature thresholds
#define LM75BD_DEFAULT_OT_THRESH 80.0f
#define LM75BD_DEFAULT_HYST_THRESH 75.0f

/**
 * @struct Configuration struct for LM75BD,118 temperature sensor
 *
 * @param devAddr I2C address of the LM75BD
 * @param osFaultQueueSize Number of consecutive OS faults until OS output is activated (1, 2, 4, or 6)
 * @param osPolarity OS output polarity, 0 = active low, 1 = active high
 * @param osOperationMode OS output operation mode, 0 = comparator, 1 = interrupt
 * @param devOperationMode Device operation mode, 0 = normal, 1 = shutdown
 * @param overTempThreshold Overtemperature shutdown threshold, in degrees Celsius
 * @param hysteresisThreshold Hysteresis threshold, in degrees Celsius
 */
typedef struct {
  uint8_t devAddr;
  uint8_t osFaultQueueSize;
  uint8_t osPolarity;
  uint8_t osOperationMode;
  uint8_t devOperationMode;
  float overTempThresholdCelsius;
  float hysteresisThresholdCelsius;
} lm75bd_config_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the LM75BD
 *
 * @param config Configuration struct for LM75BD
 * @return ERR_CODE_SUCCESS if successful, error code otherwise
 */
error_code_t lm75bdInit(lm75bd_config_t *config);

/**
 * @brief Read the temperature from the LM75BD
 *
 * @param temp Pointer to float to store the temperature in degrees Celsius
 * @return ERR_CODE_SUCCESS if successful, error code otherwise
 */
error_code_t readTempLM75BD(uint8_t devAddr, float *temp);

/**
 * @brief Write to the configuration register from the LM75BD
 *
 * @param devAddr I2C address of the LM75BD
 * @param osFaultQueueSize Number of consecutive OS faults until OS output is activated (1, 2, 4, or 6)
 * @param osPolarity OS output polarity, 0 = active low, 1 = active high
 * @param osOperationMode OS output operation mode, 0 = comparator, 1 = interrupt
 * @param devOperationMode Device operation mode, 0 = normal, 1 = shutdown
 * @return ERR_CODE_SUCCESS if successful, error code otherwise
 */
error_code_t writeConfigLM75BD(uint8_t devAddr, uint8_t osFaultQueueSize, uint8_t osPolarity,
                                   uint8_t osOperationMode, uint8_t devOperationMode);

/**
 * @brief Handle an OS interrupt from the LM75BD
 */
void osHandlerLM75BD(void);

#ifdef __cplusplus
}
#endif
