#include "main.h"
#include "system_config.h"
#include "test_fully_connected.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    UART_Config();
    
    printf("\n\r==========================================\n\r");
    printf("CMSIS-NN Fully Connected Test Suite\n\r");
    printf("==========================================\n\r");
    
    Test_FullyConnected_Run_All();
    
    printf("\n\rTests FC finalizados. Sistema en espera...\n\r");
    
    while (1) 
    { 
        // Sistema en espera
    }
}