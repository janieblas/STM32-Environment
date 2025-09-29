#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

// Callback executed if a stack overflow occurs in any task
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Option 1: enter an infinite loop for debugging
    printf("ERROR: Stack overflow in task: %s\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    for(;;);
}