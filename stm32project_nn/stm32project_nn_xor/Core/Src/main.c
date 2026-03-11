#include "main.h"
#include "system_config.h"
#include "xor_model.h"
#include "xor_float.h"

/* Performance test parameters */
#define PERFORMANCE_ITERATIONS 10000  // Más iteraciones para mejor precisión

/**
 * @brief Main application entry point
 * @return int Not used (embedded application)
 */
int main(void)
{
    /* System initialization */
    HAL_Init();
    SystemClock_Config();
    UART_Config();
    
    /* Performance measurement variables */
    uint32_t start_time, end_time;
    uint32_t cmsis_total_time = 0, float_total_time = 0;
    
    /* Application header */
    printf("\n\r==========================================\n\r");
    printf("XOR Neural Network - CMSIS-NN vs Float Comparison\n\r");
    printf("==========================================\n\r");
    
    /* Initialize both models */
    xor_model_init();
    printf("Models initialized successfully\n\r");
    
    /* Test cases for XOR */
    int8_t test_cases_quant[4][2] = {
        {0, 0},     /* 0 XOR 0 = 0 */
        {0, 127},   /* 0 XOR 1 = 1 */
        {127, 0},   /* 1 XOR 0 = 1 */
        {127, 127}  /* 1 XOR 1 = 0 */
    };
    
    float test_cases_float[4][2] = {
        {0.0f, 0.0f},   /* 0 XOR 0 = 0 */
        {0.0f, 1.0f},   /* 0 XOR 1 = 1 */
        {1.0f, 0.0f},   /* 1 XOR 0 = 1 */
        {1.0f, 1.0f}    /* 1 XOR 1 = 0 */
    };
    
    /* ================================
     * FUNCTIONALITY VERIFICATION
     * ================================ */
    printf("\n\r--- CMSIS-NN Quantized Results ---\n\r");
    for (int i = 0; i < 4; i++) 
    {
        int8_t result = xor_predict(test_cases_quant[i][0], test_cases_quant[i][1]);
        int8_t binary_result = (result > 0) ? 1 : 0;
        int expected = (test_cases_quant[i][0] != test_cases_quant[i][1]) ? 1 : 0;
        
        printf("Input: [%3d, %3d] -> Raw: %4d, Binary: %d (%s)\n\r",
               test_cases_quant[i][0], test_cases_quant[i][1],
               result, binary_result,
               (binary_result == expected) ? "PASS" : "FAIL");
    }
    
    printf("\n\r--- Float Precision Results ---\n\r");
    for (int i = 0; i < 4; i++) 
    {
        float result = xor_predict_float(test_cases_float[i][0], test_cases_float[i][1]);
        int binary_result = (result > 0.0f) ? 1 : 0;
        int expected = (test_cases_float[i][0] != test_cases_float[i][1]) ? 1 : 0;
        
        printf("Input: [%3.1f, %3.1f] -> Raw: %6.3f, Binary: %d (%s)\n\r",
               test_cases_float[i][0], test_cases_float[i][1],
               result, binary_result,
               (binary_result == expected) ? "PASS" : "FAIL");
    }
    
    /* ================================
     * PERFORMANCE COMPARISON - CORREGIDO
     * ================================ */
    printf("\n\r--- Performance Comparison (%d iterations) ---\n\r", PERFORMANCE_ITERATIONS);
    
    /* Variables para evitar optimizaciones */
    volatile int32_t cmsis_dummy_result = 0;  // Cambiado a int32_t
    volatile float float_dummy_result = 0.0f;
    
    /* Test CMSIS-NN Performance */
    start_time = HAL_GetTick();
    for (int iter = 0; iter < PERFORMANCE_ITERATIONS; iter++) 
    {
        for (int i = 0; i < 4; i++) 
        {
            int8_t result = xor_predict(test_cases_quant[i][0], test_cases_quant[i][1]);
            cmsis_dummy_result += result;  // Acumular para evitar optimización
        }
    }
    end_time = HAL_GetTick();
    cmsis_total_time = end_time - start_time;
    
    /* Pequeño delay entre tests */
    HAL_Delay(10);
    
    /* Test Float Performance */
    start_time = HAL_GetTick();
    for (int iter = 0; iter < PERFORMANCE_ITERATIONS; iter++) 
    {
        for (int i = 0; i < 4; i++) 
        {
            float result = xor_predict_float(test_cases_float[i][0], test_cases_float[i][1]);
            float_dummy_result += result;  // Acumular para evitar optimización
        }
    }
    end_time = HAL_GetTick();
    float_total_time = end_time - start_time;
    
    /* Calcular métricas CORRECTAMENTE */
    uint32_t total_inferences = PERFORMANCE_ITERATIONS * 4;
    
    uint32_t cmsis_time_per_inference = (cmsis_total_time * 1000) / total_inferences;
    uint32_t float_time_per_inference = (float_total_time * 1000) / total_inferences;
    
    // Cálculo CORRECTO del speedup
    float speedup_ratio;
    if (cmsis_total_time < float_total_time) {
        speedup_ratio = (float)float_total_time / (float)cmsis_total_time;
        printf("\n\rCMSIS-NN is %.2fx faster than Float\n\r", speedup_ratio);
    } else {
        speedup_ratio = (float)cmsis_total_time / (float)float_total_time;
        printf("\n\rFloat is %.2fx faster than CMSIS-NN\n\r", speedup_ratio);
    }
    
    /* Display performance results */
    printf("\n\rPerformance Results:\n\r");
    printf("CMSIS-NN Quantized:\n\r");
    printf("  Total time: %lu ms for %lu inferences\n\r", cmsis_total_time, total_inferences);
    printf("  Time per inference: %lu us\n\r", cmsis_time_per_inference);
    printf("  Throughput: ~%lu inferences/second\n\r", (total_inferences * 1000) / cmsis_total_time);
    
    printf("\n\rFloat Precision:\n\r");
    printf("  Total time: %lu ms for %lu inferences\n\r", float_total_time, total_inferences);
    printf("  Time per inference: %lu us\n\r", float_time_per_inference);
    printf("  Throughput: ~%lu inferences/second\n\r", (total_inferences * 1000) / float_total_time);
    
    /* ================================
     * ANÁLISIS DE LOS RESULTADOS
     * ================================ */
    printf("\n\r--- Analysis ---\n\r");
    if (float_total_time < cmsis_total_time) {
        printf("Float is faster - This can happen because:\n\r");
        printf("1. Very small network (2-2-1)\n\r");
        printf("2. CMSIS-NN has initialization overhead\n\r");
        printf("3. STM32F4 has hardware FPU\n\r");
        printf("4. CMSIS-NN optimized for larger networks\n\r");
    } else {
        printf("CMSIS-NN is faster - As expected for quantized inference\n\r");
    }
    
    /* ================================
     * MEMORY USAGE COMPARISON
     * ================================ */
    printf("\n\r--- Memory Usage Comparison ---\n\r");
    
    size_t cmsis_params_memory = sizeof(xor_weights_1) + sizeof(xor_bias_1) + 
                                sizeof(xor_weights_2) + sizeof(xor_bias_2);
    
    size_t float_params_memory = sizeof(xor_weights_1_float) + sizeof(xor_bias_1_float) + 
                                sizeof(xor_weights_2_float) + sizeof(xor_bias_2_float);
    
    printf("Parameter Memory Usage:\n\r");
    printf("  CMSIS-NN: %u bytes\n\r", (unsigned int)cmsis_params_memory);
    printf("  Float:    %u bytes\n\r", (unsigned int)float_params_memory);
    printf("  Memory reduction: %.1f%%\n\r", 
           (1.0 - (float)cmsis_params_memory / float_params_memory) * 100.0);
    
    printf("\n\rCMSIS-NN advantages become significant with:\n\r");
    printf("- Larger networks\n\r");
    printf("- Memory-constrained devices\n\r");
    printf("- No hardware FPU\n\r");
    
    /* Application footer */
    printf("\n\r==========================================\n\r");
    printf("Performance Test Completed\n\r");
    printf("System ready for operation\n\r");
    printf("==========================================\n\r");
    
    /* Usar las variables dummy para evitar warnings - CORREGIDO */
    if (cmsis_dummy_result != 0 || float_dummy_result != 0.0f) {
        printf("Debug: CMSIS=%ld, Float=%.2f\n\r", cmsis_dummy_result, float_dummy_result);
    }
    
    /* Main application loop */
    while (1) 
    {
        HAL_Delay(1000);
    }
    
    return 0;
}