#include "and_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ================================
// DEFINICIONES REALES - PARÁMETROS AND
// ================================

// Pesos AND (Q7) - [50, 50] como descubrimos
const int8_t and_weights[] = {
    50, 50  // peso1, peso2
};

// Bias AND (Q31) - -6360 como descubrimos  
const int32_t and_bias[] = {
    -6360  // Bias único
};

// Variables globales para CMSIS-NN
static cmsis_nn_context and_ctx;
static cmsis_nn_fc_params and_fc_params;
static cmsis_nn_per_tensor_quant_params and_quant_params;
static cmsis_nn_dims and_input_dims, and_filter_dims, and_bias_dims, and_output_dims;

// Buffers
static int8_t and_output[1];

void and_model_init(void)
{
    // Inicializar contexto
    and_ctx.size = 0;
    and_ctx.buf = NULL;
    
    // ================================
    // INICIALIZACIÓN CAPA AND
    // ================================
    
    and_quant_params.multiplier = 0x0204D86D; //AND_MULTIPLIER;
    and_quant_params.shift = AND_SHIFT;
    
    calculate_and_quant_params();  // Calcular automáticamente

    and_fc_params.input_offset = 0;
    and_fc_params.filter_offset = 0;
    and_fc_params.output_offset = 0;
    and_fc_params.activation.min = 0;    // ReLU - mínimo 0
    and_fc_params.activation.max = 100;  // ReLU - máximo 127, lo cambie a 100 para probar
    
    // ✅ DIMENSIONES CORRECTAS (basado en tus tests):
    and_input_dims.n = 1; and_input_dims.h = 1; and_input_dims.w = 1; and_input_dims.c = 2;
    and_filter_dims.n = 2; and_filter_dims.h = 1; and_filter_dims.w = 1; and_filter_dims.c = 1;  // ← CLAVE!
    and_bias_dims.n = 1; and_bias_dims.h = 1; and_bias_dims.w = 1; and_bias_dims.c = 1;
    and_output_dims.n = 1; and_output_dims.h = 1; and_output_dims.w = 1; and_output_dims.c = 1;
    
    // ✅ KERNEL SUM BUFFER (necesario según tests):
    static int32_t kernel_sum_buffer[1] = {0};  // Para 1 output
    and_ctx.buf = kernel_sum_buffer;
    and_ctx.size = sizeof(kernel_sum_buffer);
}

int8_t and_predict(int8_t input1, int8_t input2)
{
    int8_t input_data[2] = {input1, input2};
    
    // Capa única: 2 → 1 (ReLU)
    arm_fully_connected_s8(&and_ctx, &and_fc_params, &and_quant_params,
                          &and_input_dims, input_data,
                          &and_filter_dims, and_weights,
                          &and_bias_dims, and_bias,
                          &and_output_dims, and_output);
    
    return and_output[0];
}

void test_and_model(void) {
    printf("\n\r");
    printf("==========================================\n\r");
    printf("TEST MODELO AND EN CMSIS-NN\n\r");
    printf("==========================================\n\r");
    
    // Casos de prueba AND
    int8_t test_cases[4][2] = {
        {0, 0},      // [0,0] → esperado: 0
        {0, 127},    // [0,1] → esperado: 0
        {127, 0},    // [1,0] → esperado: 0
        {127, 127}   // [1,1] → esperado: 1
    };
    
    const char* expected[4] = {"0", "0", "0", "1"};
    
    // Debug de cálculos manuales
    debug_and_simple_calculation();
    
    // Debug de parámetros
    debug_and_quantization_params();
    
    printf("Multiplicador: 0x%X\n\r", (unsigned int)AND_MULTIPLIER);
    printf("Shift: %d\n\r", AND_SHIFT);
    
    printf("\n\r--- DEBUG DETALLADO ---\n\r");
    debug_and_internal_activations(0, 127);
    debug_and_internal_activations(127, 127);
    
    printf("\n\r--- TEST CMSIS-NN ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t result = and_predict(test_cases[i][0], test_cases[i][1]);
        int8_t binary_result = (result > 0) ? 1 : 0;
        
        printf("CMSIS-NN - Input: [%3d, %3d] → Output: %4d → Binario: %d (esperado: %s) %s\n\r",
               test_cases[i][0], test_cases[i][1], 
               result, binary_result, expected[i],
               (binary_result == atoi(expected[i])) ? "✓" : "✗");
    }

    // TEST MANUAL 
    printf("\n\r--- TEST MANUAL (sin CMSIS-NN) ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t result = manual_and_predict(test_cases[i][0], test_cases[i][1]);
        int8_t binary_result = (result > 0) ? 1 : 0;
        
        printf("MANUAL - Input: [%3d, %3d] → Output: %4d → Binario: %d (esperado: %s) %s\n\r",
               test_cases[i][0], test_cases[i][1], 
               result, binary_result, expected[i],
               (binary_result == atoi(expected[i])) ? "✓" : "✗");
    }
    
    printf("\n\r==========================================\n\r");
}

void debug_and_internal_activations(int8_t input1, int8_t input2) {
    printf("\n\r=== DEBUG AND para Input [%d, %d] ===\n\r", input1, input2);
    
    int8_t input_data[2] = {input1, input2};
    
    // Calcular manualmente
    int32_t z_quant = (int32_t)input_data[0] * (int32_t)and_weights[0] + 
                      (int32_t)input_data[1] * (int32_t)and_weights[1] + 
                      and_bias[0];
    
    printf("Cálculo manual: %d*%d + %d*%d + %ld = %ld\n\r",
           input_data[0], and_weights[0],
           input_data[1], and_weights[1], 
           (long)and_bias[0], (long)z_quant);
    
    printf("Después ReLU: %ld\n\r", (long)(z_quant > 0 ? z_quant : 0));
    printf("==============================\n\r");
}

void debug_and_quantization_params(void) {
    printf("\n\r=== DEBUG PARÁMETROS AND ===\n\r");
    printf("Pesos: [%d, %d]\n\r", and_weights[0], and_weights[1]);
    printf("Bias: %ld\n\r", (long)and_bias[0]);
    printf("Multiplier: 0x%lX\n\r", (unsigned long)and_quant_params.multiplier);
    printf("Shift: %ld\n\r", (long)and_quant_params.shift);
    printf("Activation min: %ld\n\r", (long)and_fc_params.activation.min);
    printf("Activation max: %ld\n\r", (long)and_fc_params.activation.max);
    printf("==============================\n\r");
}

void debug_and_simple_calculation(void) {
    printf("\n\r=== CÁLCULO MANUAL AND ===\n\r");
    
    // Probar el caso crítico [0,127]
    int32_t z_quant = 0 * and_weights[0] + 127 * and_weights[1] + and_bias[0];
    printf("[0,127] → 0*%d + 127*%d + %ld = %ld\n\r", 
           and_weights[0], and_weights[1], (long)and_bias[0], (long)z_quant);
    printf("Esto debería ser NEGATIVO: %s\n\r", z_quant < 0 ? "✓" : "✗");
    
    // Probar el caso [127,127]
    z_quant = 127 * and_weights[0] + 127 * and_weights[1] + and_bias[0];
    printf("[127,127] → 127*%d + 127*%d + %ld = %ld\n\r", 
           and_weights[0], and_weights[1], (long)and_bias[0], (long)z_quant);
    printf("Esto debería ser POSITIVO: %s\n\r", z_quant > 0 ? "✓" : "✗");
    printf("==============================\n\r");
}

int8_t manual_and_predict(int8_t input1, int8_t input2) {
    // Cálculo manual simple
    int32_t z_quant = (int32_t)input1 * (int32_t)and_weights[0] + 
                      (int32_t)input2 * (int32_t)and_weights[1] + 
                      and_bias[0];
    
    // Aplicar ReLU
    int32_t activated = (z_quant > 0) ? z_quant : 0;
    
    // Convertir a Q7 (aproximación)
    return (int8_t)(activated >> 8);
}

void calculate_and_quant_params(void) {
    // Calcular multiplicador y shift basado en nuestros parámetros
    // Para nuestro caso: escala = 100, necesitamos ajuste mínimo
    
    and_quant_params.multiplier = 0x7FFFFFFF;  // Máximo multiplicador
    and_quant_params.shift = 0;                // Sin shift
    
    printf("Nuevos parámetros calculados:\n\r");
    printf("Multiplier: 0x%lX\n\r", (unsigned long)and_quant_params.multiplier);
    printf("Shift: %ld\n\r", (long)and_quant_params.shift);
}