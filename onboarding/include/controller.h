#ifndef ONBOARDING_INCLUDE_CONTROLLER_H_
#define ONBOARDING_INCLUDE_CONTROLLER_H_

#include <os_projdefs.h>
#include <stdint.h>

/* Controller task config */
#define CONTROLLER_NAME         "controller"
#define CONTROLLER_STACK_SIZE   256
#define CONTROLLER_PRIORITY     1

/* Light timer config */
#define LIGHT_TIMER_NAME        "light_timer"
#define LIGHT_TIMER_PERIOD      pdMS_TO_TICKS(1000)
#define LIGHT_TIMER_AUTORELOAD  pdTRUE

/* LED timer config */
#define LED_TIMER_NAME          "led_timer"
#define LED_TIMER_PERIOD        pdMS_TO_TICKS(2500)
#define LED_TIMER_AUTORELOAD    pdTRUE

uint8_t initController(void);

#endif /* ONBOARDING_INCLUDE_CONTROLLER_H_ */