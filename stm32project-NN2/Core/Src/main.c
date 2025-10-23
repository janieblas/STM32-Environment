/**
  ******************************************************************************
  * @file    main.c 
  * @brief   Main application for XOR Neural Network
  ******************************************************************************
  */
#include "main.h"
#include "system_config.h"
#include "nn_driver.h"

/**
  * @brief  Main program
  */
int main(void)
{
    /* MCU Configuration */
    HAL_Init();
    SystemClock_Config();
    UART_Config();
    
    /* Application Header */
    printf("\n\r==========================================\n\r");
    printf("XOR Neural Network - Manual Q7\n\r");
    printf("==========================================\n\r\n\r");
    
    /* Neural Network Application */
    NN_Init();
    NN_Print_Configuration();
    NN_Test_XOR();
    
    printf("\n\rApplication running...\n\r");
    
    /* Main loop */
    while (1) 
    { 
        // Main application loop
        // Puedes agregar más lógica aquí
    }
}