#include "test_tanh_activation.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Debug del comportamiento específico de Tanh en CMSIS-NN
 */
void debug_tanh_behavior(void)
{
    printf("\n\r==========================================\n\r");
    printf("  DEBUG - Comportamiento de Tanh\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    
    // Configuración base
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x7FFFFFFF;  // 1.0
    quant_params.shift = 0;
    
    // DIMENSIONES QUE FUNCIONAN PARA AND (1 output)
    cmsis_nn_dims input_dims = {1, 1, 1, 2};     // 2 inputs
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};    // ¡ESTRUCTURA CLAVE!
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    int8_t input[2] = {0};
    int8_t weights[2] = {1, 1};  // Suma los inputs
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    printf("Configuración AND (1 output):\n\r");
    printf("  Filter: n=2, h=1, w=1, c=1\n\r");
    printf("  Interpretación: n=input_channels, c=output_channels\n\r\n\r");
    
    // Probar diferentes valores con Tanh
    printf("=== PROBAR TANH CON DIFERENTES VALORES ===\n\r");
    
struct {
    int8_t input1, input2;
    const char* desc;
} test_cases[] = {
    {10, 10, NULL},
    {50, 50, NULL},  
    {100, 100, NULL},
    {-10, -10, NULL},
    {-50, -50, NULL},
    {-100, -100, NULL},
    {100, -100, NULL},
};
    
    for(int i = 0; i < 7; i++) {
        // TEST CON ReLU (referencia)
        fc_params.activation.min = 0;
        fc_params.activation.max = 127;
        
        input[0] = test_cases[i].input1;
        input[1] = test_cases[i].input2;
        
        arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                              &input_dims, input, &filter_dims, weights,
                              &bias_dims, bias, &output_dims, output);
        
        int8_t relu_output = output[0];
        
        // TEST CON Tanh
        fc_params.activation.min = -128;
        fc_params.activation.max = 127;
        
        arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                              &input_dims, input, &filter_dims, weights,
                              &bias_dims, bias, &output_dims, output);
        
        int8_t tanh_output = output[0];
        
        printf("Input: [%4d, %4d] → ", test_cases[i].input1, test_cases[i].input2);
        printf("ReLU: %4d, Tanh: %4d", relu_output, tanh_output);
        
        // Análisis del comportamiento
        if(tanh_output == 0 && (test_cases[i].input1 + test_cases[i].input2) != 0) {
            printf(" ⚠️ Tanh=0 (¿bug?)\n\r");
        } else if(tanh_output > 0 && relu_output > 0) {
            printf(" ✓ Ambos positivos\n\r");
        } else if(tanh_output < 0 && relu_output == 0) {
            printf(" ✓ Tanh negativo, ReLU=0\n\r");
        } else {
            printf(" ? Comportamiento inesperado\n\r");
        }
    }
}

/**
 * @brief Test específico para valores positivos vs negativos
 */
void test_tanh_positive_negative(void)
{
    printf("\n\r==========================================\n\r");
    printf("  TEST - Tanh: Positivos vs Negativos\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0x7FFFFFFF, 0};
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // DIMENSIONES CLAVE (como en AND)
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};  // Estructura mágica
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    int8_t input[2] = {0};
    int8_t weights[2] = {1, 1};
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    printf("Probando el rango completo...\n\r\n\r");
    
    // Probar valores específicos que deberían dar Tanh positivo
    struct {
        int8_t input1, input2;
        int32_t expected_sum;
        const char* scenario;
    } critical_cases[] = {
        {1, 1, 2, "Mínimo positivo"},
        {10, 10, 20, "Pequeño positivo"},
        {50, 50, 100, "Mediano positivo"},
        {100, 27, 127, "Borde positivo (127)"},
        {127, 0, 127, "Máximo input"},
        {-1, -1, -2, "Mínimo negativo"},
        {-10, -10, -20, "Pequeño negativo"},
        {-50, -50, -100, "Mediano negativo"},
        {-100, -28, -128, "Borde negativo (-128)"},
        {-127, 0, -127, "Mínimo input"},
    };
    
    for(int i = 0; i < 10; i++) {
        input[0] = critical_cases[i].input1;
        input[1] = critical_cases[i].input2;
        
        arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                              &input_dims, input, &filter_dims, weights,
                              &bias_dims, bias, &output_dims, output);
        
        printf("%s: [%4d, %4d] → ", critical_cases[i].scenario,
               critical_cases[i].input1, critical_cases[i].input2);
        printf("Suma=%5ld → Tanh=%4d", 
               (long)critical_cases[i].expected_sum, output[0]);
        
        // Verificar comportamiento esperado
        if(critical_cases[i].expected_sum > 0 && output[0] > 0) {
            printf(" ✓ Positivo correcto\n\r");
        } else if(critical_cases[i].expected_sum < 0 && output[0] < 0) {
            printf(" ✓ Negativo correcto\n\r");
        } else if(critical_cases[i].expected_sum == 0 && output[0] == 0) {
            printf(" ✓ Cero correcto\n\r");
        } else {
            printf(" ✗ COMPORTAMIENTO INESPERADO\n\r");
        }
    }
}

/**
 * @brief Test de saturación en Tanh
 */
void test_tanh_saturation(void)
{
    printf("\n\r==========================================\n\r");
    printf("  TEST - Saturación Tanh\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0x7FFFFFFF, 0};
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    int8_t input[2] = {0};
    int8_t weights[2] = {1, 1};
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    printf("Probando saturación...\n\r\n\r");
    
    // Valores que deberían saturar
    struct {
        int8_t input1, input2;
        int8_t expected_tanh;
        const char* description;
    } saturation_cases[] = {
        {127, 127, 127, "Máxima saturación positiva"},
        {100, 100, 127, "Saturación positiva"},
        {-128, -128, -128, "Máxima saturación negativa"},
        {-100, -100, -128, "Saturación negativa"},
        {127, 0, 127, "Saturación con un input máximo"},
        {-128, 0, -128, "Saturación con un input mínimo"},
    };
    
    for(int i = 0; i < 6; i++) {
        input[0] = saturation_cases[i].input1;
        input[1] = saturation_cases[i].input2;
        
        arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                              &input_dims, input, &filter_dims, weights,
                              &bias_dims, bias, &output_dims, output);
        
        printf("%s: [%4d, %4d] → ", saturation_cases[i].description,
               saturation_cases[i].input1, saturation_cases[i].input2);
        printf("Tanh=%4d (esperado: %4d) → %s\n\r",
               output[0], saturation_cases[i].expected_tanh,
               (output[0] == saturation_cases[i].expected_tanh) ? "✓" : "✗");
    }
}

/**
 * @brief Comparación directa Tanh vs ReLU
 */
void test_tanh_vs_relu_comparison(void)
{
    printf("\n\r==========================================\n\r");
    printf("  TEST - Comparación Tanh vs ReLU\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0x7FFFFFFF, 0};
    
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    int8_t input[2] = {50, 50};  // Caso que debería funcionar
    int8_t weights[2] = {1, 1};
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    printf("Mismo cálculo, diferentes activaciones:\n\r");
    printf("Input: [50, 50], Weights: [1, 1], Bias: 0\n\r");
    printf("Cálculo: 50×1 + 50×1 = 100\n\r\n\r");
    
    // Test ReLU
    fc_params.activation.min = 0;
    fc_params.activation.max = 127;
    arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                          &input_dims, input, &filter_dims, weights,
                          &bias_dims, bias, &output_dims, output);
    printf("ReLU  (0,127):   %4d\n\r", output[0]);
    
    // Test Tanh
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                          &input_dims, input, &filter_dims, weights,
                          &bias_dims, bias, &output_dims, output);
    printf("Tanh (-128,127): %4d\n\r", output[0]);
    
    // Test Sin activación (solo quant)
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    quant_params.multiplier = 0x00000001;  // Sin quantización
    quant_params.shift = 0;
    arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                          &input_dims, input, &filter_dims, weights,
                          &bias_dims, bias, &output_dims, output);
    printf("Sin Quant:       %4d\n\r", output[0]);
    
    printf("\n\rAnálisis:\n\r");
    if(output[0] == 100) {
        printf("✓ Sin quantización funciona\n\r");
        printf("⚠️ El problema puede estar en los multiplicadores\n\r");
    } else {
        printf("✗ Incluso sin quantización falla\n\r");
        printf("⚠️ El problema es más fundamental\n\r");
    }
}

/**
 * @brief Test con diferentes multiplicadores
 */
void test_tanh_with_different_multipliers(void)
{
    printf("\n\r==========================================\n\r");
    printf("  TEST - Tanh con Diferentes Multiplicadores\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    int8_t input[2] = {50, 50};
    int8_t weights[2] = {1, 1};
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    printf("Input: [50, 50] → Esperado: ~100\n\r\n\r");
    
    struct {
        uint32_t multiplier;
        int8_t shift;
        const char* desc;
    } multipliers[] = {
        {0x7FFFFFFF, 0, "1.0 (máximo)"},
        {0x40000000, 1, "0.5 con shift 1"},
        {0x20000000, 2, "0.25 con shift 2"},
        {0x10000000, 3, "0.125 con shift 3"},
        {0x00000001, 0, "Mínimo (casi cero)"},
        {0x0CCCCCCC, 0, "0.1 (Python XOR capa1)"},
        {0x050F6F12, 0, "0.039534 (Python XOR capa2)"},
    };
    
    for(int i = 0; i < 7; i++) {
        quant_params.multiplier = multipliers[i].multiplier;
        quant_params.shift = multipliers[i].shift;
        
        arm_fully_connected_s8(&ctx, &fc_params, &quant_params,
                              &input_dims, input, &filter_dims, weights,
                              &bias_dims, bias, &output_dims, output);
        
        printf("%s: %4d → ", multipliers[i].desc, output[0]);
        
        if(output[0] > 0) {
            printf("✓ Positivo\n\r");
        } else if(output[0] < 0) {
            printf("✗ Negativo (ERROR)\n\r");
        } else {
            printf("⚠️ Cero (¿underflow?)\n\r");
        }
    }
}

/**
 * @brief Run all Tanh tests
 */
void Test_Tanh_Activation_Run_All(void)
{
    printf("\n\r");
    printf("=========================================================\n\r");
    printf("  Test Suite ESPECÍFICO - Tanh en arm_fully_connected_s8()\n\r");
    printf("=========================================================\n\r");
    printf("\n\r");
    
    debug_tanh_behavior();
    test_tanh_positive_negative();
    test_tanh_saturation();
    test_tanh_vs_relu_comparison();
    test_tanh_with_different_multipliers();
    
    printf("\n\r");
    printf("=========================================================\n\r");
    printf("  Tests Tanh completados!\n\r");
    printf("=========================================================\n\r");
    printf("\n\r");
}