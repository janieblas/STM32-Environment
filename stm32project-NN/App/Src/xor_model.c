#include "xor_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ================================
// PARÁMETROS XOR - SOLO CAPA 1
// ================================

// Capa 1: 2→2 (Q7) - USAR VALORES ORIGINALES DE PYTHON:
const int8_t xor_weights_1[] = {
    20, -20,   // Neurona 1: [20, -20] - VALORES ORIGINALES
    -20, 20    // Neurona 2: [-20, 20] - VALORES ORIGINALES
};

// Bias capa 1 (Q31) - USAR VALORES ORIGINALES:
const int32_t xor_bias_1[] = {
    -10, -10   // Bias original de Python
};

// Contextos CMSIS-NN (solo capa 1)
static cmsis_nn_context xor_ctx_1;
static cmsis_nn_fc_params xor_fc_params_1;
static cmsis_nn_per_tensor_quant_params xor_quant_params_1;
static cmsis_nn_dims xor_input_dims, xor_hidden_dims;
static cmsis_nn_dims xor_filter_dims_1, xor_bias_dims_1;

//======================================================================

// Capa 2: 2→1 (Q7) - VALORES DE PYTHON
const int8_t xor_weights_2[] = {
    10, 10     // Pesos: [10, 10] - VALORES ORIGINALES
};

// Bias capa 2 (Q31) - VALOR DE PYTHON
const int32_t xor_bias_2[] = {
    -5         // Bias único original
};

// Contexto capa 2
static cmsis_nn_context xor_ctx_2;
static cmsis_nn_fc_params xor_fc_params_2;
static cmsis_nn_per_tensor_quant_params xor_quant_params_2;
static cmsis_nn_dims xor_filter_dims_2, xor_bias_dims_2, xor_output_dims;
static int8_t xor_final_output[1];

void calculate_xor_quant_params(void) {
    // USAR MULTIPLICADOR DE PYTHON para capa 1
    xor_quant_params_1.multiplier = XOR_MULTIPLIER_1;  // 0.100000 de Python
    xor_quant_params_1.shift = XOR_SHIFT_1;
    
    printf("Parámetros Capa 1 calculados (Python):\n\r");
    printf("Multiplier Capa 1: 0x%lX\n\r", (unsigned long)xor_quant_params_1.multiplier);
}

void xor_model_init(void)
{
    // ================================
    // INICIALIZACIÓN SOLO CAPA 1 (2→2)
    // ================================
    
    // Contexto capa 1
    xor_ctx_1.size = 0;
    xor_ctx_1.buf = NULL;
    
    // Parámetros cuantización capa 1
    calculate_xor_quant_params();

    // Parámetros fully connected capa 1
    xor_fc_params_1.input_offset = 0;
    xor_fc_params_1.filter_offset = 0;
    xor_fc_params_1.output_offset = 0;
    xor_fc_params_1.activation.min = 0;    // ReLU
    xor_fc_params_1.activation.max = 127;  // Máximo Q7
    
    // Dimensiones capa 1
    xor_input_dims.n = 1; xor_input_dims.h = 1; xor_input_dims.w = 1; xor_input_dims.c = 2;
    xor_filter_dims_1.n = 2; xor_filter_dims_1.h = 1; xor_filter_dims_1.w = 1; xor_filter_dims_1.c = 2;
    xor_bias_dims_1.n = 1; xor_bias_dims_1.h = 1; xor_bias_dims_1.w = 1; xor_bias_dims_1.c = 2;
    xor_hidden_dims.n = 1; xor_hidden_dims.h = 1; xor_hidden_dims.w = 1; xor_hidden_dims.c = 2;
    
    // Buffer para suma de kernel capa 1
    static int32_t kernel_sum_buffer_1[2] = {0};
    xor_ctx_1.buf = kernel_sum_buffer_1;
    xor_ctx_1.size = sizeof(kernel_sum_buffer_1);

    // ================================
    // INICIALIZACIÓN CAPA 2 (2→1)
    // ================================
    
    // Contexto capa 2
    xor_ctx_2.size = 0;
    xor_ctx_2.buf = NULL;
    
    // Parámetros cuantización capa 2
    xor_quant_params_2.multiplier = XOR_MULTIPLIER_2;
    xor_quant_params_2.shift = XOR_SHIFT_2;
    
    // Parámetros fully connected capa 2
    xor_fc_params_2.input_offset = 0;
    xor_fc_params_2.filter_offset = 0;
    xor_fc_params_2.output_offset = 0;
    xor_fc_params_2.activation.min = -128;    // Tanh
    xor_fc_params_2.activation.max = 127;
    
    // Dimensiones capa 2
    // Dimensiones capa 2 - USAR ESTRUCTURA AND:
    xor_filter_dims_2.n = 2; xor_filter_dims_2.h = 1; xor_filter_dims_2.w = 1; xor_filter_dims_2.c = 1;  // ← CLAVE!
    xor_bias_dims_2.n = 1; xor_bias_dims_2.h = 1; xor_bias_dims_2.w = 1; xor_bias_dims_2.c = 1;
    xor_output_dims.n = 1; xor_output_dims.h = 1; xor_output_dims.w = 1; xor_output_dims.c = 1;
    
    // Buffer para suma de kernel capa 2
    static int32_t kernel_sum_buffer_2[1] = {0};
    xor_ctx_2.buf = kernel_sum_buffer_2;
    xor_ctx_2.size = sizeof(kernel_sum_buffer_2);
}

// Función para obtener solo salida de capa 1
void get_xor_layer1_output(int8_t input1, int8_t input2, int8_t* hidden_output) {
    int8_t input_data[2] = {input1, input2};
    
    // Solo capa 1: 2→2 (ReLU)
    arm_fully_connected_s8(&xor_ctx_1, &xor_fc_params_1, &xor_quant_params_1,
                          &xor_input_dims, input_data,
                          &xor_filter_dims_1, xor_weights_1,
                          &xor_bias_dims_1, xor_bias_1,
                          &xor_hidden_dims, hidden_output);
}

void test_xor_layer1_only(void) {
    printf("\n\r");
    printf("==========================================\n\r");
    printf("TEST SOLO CAPA OCULTA XOR (2→2)\n\r");
    printf("==========================================\n\r");
    
    // Casos de prueba XOR
    int8_t test_cases[4][2] = {
        {0, 0},      // [0,0] → esperado: [0, 0]
        {0, 127},    // [0,1] → esperado: [0, POSITIVO]
        {127, 0},    // [1,0] → esperado: [POSITIVO, 0]
        {127, 127}   // [1,1] → esperado: [0, 0]
    };
    
    const char* expected[4] = {"[0,0]", "[0,+]", "[+,0]", "[0,0]"};
    
    // Debug de parámetros
    debug_xor_quantization_params();
    
    printf("Multiplicador Capa 1: 0x%X\n\r", (unsigned int)XOR_MULTIPLIER_1);
    
    printf("\n\r--- DEBUG DETALLADO CAPA 1 ---\n\r");
    debug_xor_layer1_activations(0, 127);
    debug_xor_layer1_activations(127, 127);
    
    printf("\n\r--- TEST CMSIS-NN CAPA 1 ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t hidden_output[2];
        get_xor_layer1_output(test_cases[i][0], test_cases[i][1], hidden_output);
        
        printf("Capa 1 - Input: [%3d, %3d] → Hidden: [%4d, %4d] (esperado: %s) ",
               test_cases[i][0], test_cases[i][1], 
               hidden_output[0], hidden_output[1], expected[i]);
        
        // Verificar resultados
        int correct = 1;
        if(i == 0 && (hidden_output[0] != 0 || hidden_output[1] != 0)) correct = 0;
        if(i == 1 && (hidden_output[0] != 0 || hidden_output[1] <= 0)) correct = 0;
        if(i == 2 && (hidden_output[0] <= 0 || hidden_output[1] != 0)) correct = 0;
        if(i == 3 && (hidden_output[0] != 0 || hidden_output[1] != 0)) correct = 0;
        
        printf("%s\n\r", correct ? "✓" : "✗");
    }

    // TEST MANUAL CAPA 1
    printf("\n\r--- TEST MANUAL CAPA 1 (sin CMSIS-NN) ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t result0, result1;
        manual_xor_layer1_predict(test_cases[i][0], test_cases[i][1], &result0, &result1);
        
        printf("MANUAL - Input: [%3d, %3d] → Hidden: [%4d, %4d] (esperado: %s) ",
               test_cases[i][0], test_cases[i][1], result0, result1, expected[i]);
        
        // Verificar resultados
        int correct = 1;
        if(i == 0 && (result0 != 0 || result1 != 0)) correct = 0;
        if(i == 1 && (result0 != 0 || result1 <= 0)) correct = 0;
        if(i == 2 && (result0 <= 0 || result1 != 0)) correct = 0;
        if(i == 3 && (result0 != 0 || result1 != 0)) correct = 0;
        
        printf("%s\n\r", correct ? "✓" : "✗");
    }
    
    printf("\n\r==========================================\n\r");
}

// Función completa de predicción
int8_t xor_predict(int8_t input1, int8_t input2)
{
    int8_t hidden_output[2];
    
    // Capa 1: 2→2 (ReLU)
    get_xor_layer1_output(input1, input2, hidden_output);
    
    // Capa 2: 2→1 (Tanh)
    arm_fully_connected_s8(&xor_ctx_2, &xor_fc_params_2, &xor_quant_params_2,
                          &xor_hidden_dims, hidden_output,
                          &xor_filter_dims_2, xor_weights_2,
                          &xor_bias_dims_2, xor_bias_2,
                          &xor_output_dims, xor_final_output);
    
    return xor_final_output[0];
}

void debug_xor_layer1_activations(int8_t input1, int8_t input2) {
    printf("\n\r=== DEBUG CAPA 1 para Input [%d, %d] ===\n\r", input1, input2);
    
    int8_t input_data[2] = {input1, input2};
    
    // Calcular capa 1 manualmente con VALORES ORIGINALES
    int32_t z1_0 = (int32_t)input_data[0] * 20 + (int32_t)input_data[1] * (-20) - 10;
    int32_t z1_1 = (int32_t)input_data[0] * (-20) + (int32_t)input_data[1] * 20 - 10;
    
    int32_t a1_0 = (z1_0 > 0) ? z1_0 : 0;
    int32_t a1_1 = (z1_1 > 0) ? z1_1 : 0;
    
    printf("Neurona 0: %d*20 + %d*(-20) - 10 = %ld → ReLU: %ld\n\r",
           input_data[0], input_data[1], (long)z1_0, (long)a1_0);
           
    printf("Neurona 1: %d*(-20) + %d*20 - 10 = %ld → ReLU: %ld\n\r",
           input_data[0], input_data[1], (long)z1_1, (long)a1_1);
    
    printf("Salida esperada: [%ld, %ld]\n\r", (long)a1_0, (long)a1_1);
    printf("==============================\n\r");
}

void debug_xor_quantization_params(void) {
    printf("\n\r=== DEBUG PARÁMETROS CAPA 1 ===\n\r");
    printf("Pesos Capa 1: [%d, %d, %d, %d]\n\r", 
           xor_weights_1[0], xor_weights_1[1], xor_weights_1[2], xor_weights_1[3]);
    printf("Bias Capa 1: [%ld, %ld]\n\r", (long)xor_bias_1[0], (long)xor_bias_1[1]);
    printf("Multiplier Capa 1: 0x%lX\n\r", (unsigned long)xor_quant_params_1.multiplier);
    printf("Shift Capa 1: %ld\n\r", (long)xor_quant_params_1.shift);
    printf("==============================\n\r");
}

// Función manual para capa 1
void manual_xor_layer1_predict(int8_t input1, int8_t input2, int8_t* output0, int8_t* output1) {
    int8_t input_data[2] = {input1, input2};
    
    // Capa 1 - ReLU con VALORES ORIGINALES
    int32_t z1_0 = (int32_t)input_data[0] * 20 + (int32_t)input_data[1] * (-20) - 10;
    int32_t z1_1 = (int32_t)input_data[0] * (-20) + (int32_t)input_data[1] * 20 - 10;
    
    int32_t a1_0 = (z1_0 > 0) ? z1_0 : 0;
    int32_t a1_1 = (z1_1 > 0) ? z1_1 : 0;
    
    // CMSIS-NN aplica el multiplicador 0x0CCCCCCC (≈0.1) 
    // Para 2530 * 0.1 ≈ 253, pero se satura a 127 en Q7
    // Para nuestros valores: cualquier cosa > 1270 se satura a 127
    
    *output0 = (a1_0 > 1270) ? 127 : (int8_t)(a1_0 / 10);  // Aproximación
    *output1 = (a1_1 > 1270) ? 127 : (int8_t)(a1_1 / 10);
}

void debug_xor_complete_flow(int8_t input1, int8_t input2) {
    printf("\n\r=== DEBUG FLUJO COMPLETO [%d, %d] ===\n\r", input1, input2);
    
    int8_t hidden_output[2];
    
    // Obtener salida capa 1
    get_xor_layer1_output(input1, input2, hidden_output);
    printf("Capa 1 → Hidden: [%d, %d]\n\r", hidden_output[0], hidden_output[1]);
    
    // Calcular manualmente capa 2
    int32_t z2_manual = (int32_t)hidden_output[0] * 10 + (int32_t)hidden_output[1] * 10 - 5;
    printf("Capa 2 manual: %d*10 + %d*10 - 5 = %ld\n\r", 
           hidden_output[0], hidden_output[1], (long)z2_manual);
    
    // Obtener salida capa 2 con CMSIS-NN
    int8_t final_output;
    arm_fully_connected_s8(&xor_ctx_2, &xor_fc_params_2, &xor_quant_params_2,
                          &xor_hidden_dims, hidden_output,
                          &xor_filter_dims_2, xor_weights_2,
                          &xor_bias_dims_2, xor_bias_2,
                          &xor_output_dims, &final_output);
    
    printf("CMSIS-NN → Output: %d\n\r", final_output);
    printf("==============================\n\r");
}

// Test del modelo completo
void test_xor_complete_model(void) {
    printf("\n\r==========================================\n\r");
    printf("TEST MODELO XOR COMPLETO\n\r");
    printf("==========================================\n\r");
    
    int8_t test_cases[4][2] = {
        {0, 0}, {0, 127}, {127, 0}, {127, 127}
    };
    
    const char* expected[4] = {"0", "1", "1", "0"};
    
    // Primero debug del flujo completo
    printf("\n\r--- DEBUG FLUJO COMPLETO ---\n\r");
    for(int i = 0; i < 4; i++) {
        debug_xor_complete_flow(test_cases[i][0], test_cases[i][1]);
    }
    
    // Luego test normal
    printf("\n\r--- TEST XOR COMPLETO ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t result = xor_predict(test_cases[i][0], test_cases[i][1]);
        int8_t binary_result = (result > 0) ? 1 : 0;
        
        printf("XOR - Input: [%3d, %3d] → Output: %4d → Binario: %d (esperado: %s) %s\n\r",
               test_cases[i][0], test_cases[i][1], 
               result, binary_result, expected[i],
               (binary_result == atoi(expected[i])) ? "✓" : "✗");
    }
    printf("\n\r==========================================\n\r");
}