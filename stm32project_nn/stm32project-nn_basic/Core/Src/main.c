#include "main.h"
#include "system_config.h"
#include "xor_model.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    UART_Config();
    
    printf("\n\r==========================================\n\r");
    printf("TEST MODELO XOR COMPLETO\n\r");
    printf("==========================================\n\r");
    
    // Inicializar el modelo XOR
    xor_model_init();
    
    // 1. Probar solo capa oculta
    test_xor_layer1_only();
    
    // 2. Probar modelo completo
    test_xor_complete_model();
    
    printf("\n\r==========================================\n\r");
    printf("TEST COMPLETADO\n\r");
    printf("==========================================\n\r");
    
    printf("\n\r Sistema en espera...\n\r");
    
    while (1) 
    { 
        // Sistema en espera
    }
}