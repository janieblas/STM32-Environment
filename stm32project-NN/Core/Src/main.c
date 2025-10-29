#include "main.h"
#include "system_config.h"
#include "and_model.h"  // Cambiar por AND

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    UART_Config();
    
    printf("\n\r==========================================\n\r");
    printf("CMSIS-NN AND Model - Basado en aprendizaje\n\r");
    printf("==========================================\n\r");
    
    and_model_init();
    test_and_model();

    printf("\n\r Sistema en espera...\n\r");
    
    while (1) 
    { 
        // Sistema en espera
    }
}