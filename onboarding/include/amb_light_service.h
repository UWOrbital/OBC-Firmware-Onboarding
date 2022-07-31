#ifndef ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_
#define ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_

#include <stdint.h>

typedef enum {
    MEASURE_LIGHT
} light_event_t;

#define LIGHT_SERVICE_NAME "light_service"
#define LIGHT_SERVICE_STACK_SIZE 256
#define LIGHT_SERVICE_PRIORITY 1

#define LIGHT_SERVICE_QUEUE_LENGTH 10
#define LIGHT_SERVICE_QUEUE_ITEM_SIZE sizeof(light_event_t)

uint8_t initLightService(void);

uint8_t sendToLightServiceQueue(light_event_t *event);

#endif /* ONBOARDING_INCLUDE_AMB_LIGHT_SERVICE_H_ */