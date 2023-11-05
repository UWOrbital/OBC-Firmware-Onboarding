#include "controller.h"

#include <FreeRTOS.h>
#include <os_task.h>

int main(void) {
  initController();
  
  vTaskStartScheduler();

  return 0;
}
