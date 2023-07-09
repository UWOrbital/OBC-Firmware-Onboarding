#include "console.h"

#include <FreeRTOS.h>
#include <os_task.h>

void sampleTask(void *arg) {
  while (1) {
    printConsole("Hello, world! %d\n", 4);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int main(void) {
  initConsole();

  TaskHandle_t taskHandle = {0};
  xTaskCreate(sampleTask, "Sample Task", 1024, NULL, 1, &taskHandle);
  
  vTaskStartScheduler();

  return 0;
}
