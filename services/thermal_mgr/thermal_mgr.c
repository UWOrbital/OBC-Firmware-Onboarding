#include "thermal_mgr.h"
#include "errors.h"
#include "lm75bd.h"
#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>
#include <os_queue.h>

#include <string.h>

#define THERMAL_MGR_STACK_SIZE 256U

static TaskHandle_t thermalMgrTaskHandle;
static StaticTask_t thermalMgrTaskBuffer;
static StackType_t thermalMgrTaskStack[THERMAL_MGR_STACK_SIZE];

#define THERMAL_MGR_QUEUE_LENGTH 10U
#define THERMAL_MGR_QUEUE_ITEM_SIZE sizeof(thermal_mgr_event_t)

static QueueHandle_t thermalMgrQueueHandle;
static StaticQueue_t thermalMgrQueueBuffer;
static uint8_t thermalMgrQueueStorageArea[THERMAL_MGR_QUEUE_LENGTH * THERMAL_MGR_QUEUE_ITEM_SIZE];

static void thermalMgr(void *pvParameters);

static volatile uint8_t recomputeOTStateFlag = 0;    

void initThermalSystemManager(lm75bd_config_t *config) {
  memset(&thermalMgrTaskBuffer, 0, sizeof(thermalMgrTaskBuffer));
  memset(thermalMgrTaskStack, 0, sizeof(thermalMgrTaskStack));
  
  thermalMgrTaskHandle = xTaskCreateStatic(
    thermalMgr, "thermalMgr", THERMAL_MGR_STACK_SIZE,
    config, 1, thermalMgrTaskStack, &thermalMgrTaskBuffer);

  memset(&thermalMgrQueueBuffer, 0, sizeof(thermalMgrQueueBuffer));
  memset(thermalMgrQueueStorageArea, 0, sizeof(thermalMgrQueueStorageArea));

  thermalMgrQueueHandle = xQueueCreateStatic(
    THERMAL_MGR_QUEUE_LENGTH, THERMAL_MGR_QUEUE_ITEM_SIZE,
    thermalMgrQueueStorageArea, &thermalMgrQueueBuffer);

}

/*
 * @brief Sends event to the thermal manager queue. Fails is queue is full.
 *
 * @param event - Pointer to a thermal_mgr_event_t to add to the queue.
 */
error_code_t thermalMgrSendEvent(thermal_mgr_event_t *event) {
    /* Send an event to the thermal manager queue */    
    // Check for invalid parameter
    if( event == NULL ) {
        return ERR_CODE_INVALID_ARG;
    }
    // Check if the queue has been created
    if( thermalMgrQueueHandle == NULL ) {
        return ERR_CODE_UNKNOWN;
    }

    switch( xQueueSend( thermalMgrQueueHandle, (void *) event, 0 ) ) {
        case pdTRUE:
            return ERR_CODE_SUCCESS; 
            break;
        case errQUEUE_FULL:
            return ERR_CODE_QUEUE_FULL;    
            break;
        default:
            return ERR_CODE_UNKNOWN;
    }
}
/* 
 * @brief Handles overtemperature events from temperature sensor. 
 */
void osHandlerLM75BD() {    
    recomputeOTStateFlag = 1;
    // Reset overtemperature   
    thermal_mgr_event_t resetEvent;
    resetEvent.type = THERMAL_MGR_EVENT_MEASURE_TEMP_CMD;  

    thermalMgrSendEvent( &resetEvent );   
}

/*
 * @brief Task responsible for parsing events in its queue 
 *
 * @param pvParameters - Configuration struct for the LM75BD temp sensor 
 */
static void thermalMgr(void *pvParameters) {      
    // XXX: It is assumed we don't have to initialize temperature sensor
    lm75bd_config_t *temp_sensor_config;  
    error_code_t errCode;
    temp_sensor_config = (lm75bd_config_t *) pvParameters;   

    thermal_mgr_event_t event_recieved;
    while( xQueueReceive( thermalMgrQueueHandle, &event_recieved, 0) == pdTRUE ) {    
            // We got an event from the thermalMgr queue, parse it
        switch( event_recieved.type ){  
            case THERMAL_MGR_EVENT_MEASURE_TEMP_CMD:    
                    // Read temperature 
                    float temperatureResult = 0.0;   
                    errCode = readTempLM75BD( temp_sensor_config->devAddr, &temperatureResult );  
                   
                    if ( errCode != ERR_CODE_SUCCESS ) {
                        // We tried to read temperature, and something bad happened, break early
                        break;
                    }
                    // Only change overtemperature state if we had an interrupt raised recently
                    if (recomputeOTStateFlag) {  

                        if ( temperatureResult > 80.0 ) { 
                            overTemperatureDetected();
                        } else {
                            safeOperatingConditions();
                        }  
                        // We've handled the flag, reset it
                        recomputeOTStateFlag = 0;
                    }

                    addTemperatureTelemetry( temperatureResult );
                break; 
            default:
                // We got an event we weren't expecting, do nothing
                break; 
        }
    }
}

void addTemperatureTelemetry(float tempC) {
  printConsole("Temperature telemetry: %f deg C\n", tempC);
}

void overTemperatureDetected(void) {
  printConsole("Over temperature detected!\n");
}

void safeOperatingConditions(void) { 
  printConsole("Returned to safe operating conditions!\n");
}
