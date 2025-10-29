#include "xor_model.h"
#include <stdio.h>
#include <stdlib.h>  // Add this for atoi()
#include <stdint.h>  // Add this for uint32_t

#include "arm_nnfunctions.h"

// ================================
// DEFINICIONES REALES (en el .c file)
// ================================

// Pesos capa oculta (Q7)
// Pesos capa oculta (Q7) - REORGANIZADOS
const int8_t hidden_weights[] = {
    // Formato: [output1_input1, output1_input2, output2_input1, output2_input2, ...]
     78, -96, -96,  49, 113, -100, -96, -96,   // Todos los pesos para input 0
    112,  96,  96, 127, -75,  -92,  97,  97    // Todos los pesos para input 1
};

// Bias capa oculta (Q31)
const int32_t hidden_bias[] = {
     -9863,   // Neurona 0
       -16,   // Neurona 1
       -13,   // Neurona 2
     -6214,   // Neurona 3
        74,   // Neurona 4
     12740,   // Neurona 5
       -89,   // Neurona 6
       -89    // Neurona 7
};

// Pesos capa salida (Q7)
const int8_t output_weights[] = {
    -106,   // Neurona 0
     104,   // Neurona 1
      72,   // Neurona 2
     -96,   // Neurona 3
     119,   // Neurona 4
    -127,   // Neurona 5
     123,   // Neurona 6
      75    // Neurona 7
};

// Bias capa salida (Q31)
const int32_t output_bias[] = {
    0  // Bias capa salida
};

// Variables globales para CMSIS-NN
static cmsis_nn_context ctx;
static cmsis_nn_fc_params hidden_fc_params, output_fc_params;
static cmsis_nn_per_tensor_quant_params hidden_quant_params, output_quant_params;
static cmsis_nn_dims hidden_input_dims, hidden_filter_dims, hidden_bias_dims, hidden_output_dims;
static cmsis_nn_dims output_input_dims, output_filter_dims, output_bias_dims, output_dims;

// Buffers
static int8_t hidden_output[8];
static int8_t final_output[1];

void xor_model_init(void)
{
    // Inicializar contexto
    ctx.size = 0;
    ctx.buf = NULL;
    
    // ================================
    // INICIALIZACIÓN CAPA OCULTA - CORREGIDO
    // ================================
    
    hidden_quant_params.multiplier = HIDDEN_MULTIPLIER;
    hidden_quant_params.shift = HIDDEN_SHIFT;
    
    hidden_fc_params.input_offset = 0;
    hidden_fc_params.filter_offset = 0;
    hidden_fc_params.output_offset = 0;
    hidden_fc_params.activation.min = -32;
    hidden_fc_params.activation.max = 32;
    
    // ✅ DIMENSIONES CORRECTAS:
    hidden_input_dims.n = 1; hidden_input_dims.h = 1; hidden_input_dims.w = 1; hidden_input_dims.c = 2;
    hidden_filter_dims.n = 8; hidden_filter_dims.h = 2; hidden_filter_dims.w = 1; hidden_filter_dims.c = 1;  // 8 outputs, 2 inputs
    hidden_bias_dims.n = 1; hidden_bias_dims.h = 1; hidden_bias_dims.w = 1; hidden_bias_dims.c = 8;
    hidden_output_dims.n = 1; hidden_output_dims.h = 1; hidden_output_dims.w = 1; hidden_output_dims.c = 8;
    
    // ================================
    // INICIALIZACIÓN CAPA SALIDA - CORREGIDO
    // ================================
    
    output_quant_params.multiplier = OUTPUT_MULTIPLIER;
    output_quant_params.shift = OUTPUT_SHIFT;
    
    output_fc_params.input_offset = 0;
    output_fc_params.filter_offset = 0;
    output_fc_params.output_offset = 0;
    output_fc_params.activation.min = -16;
    output_fc_params.activation.max = 16;
    
    // ✅ DIMENSIONES CORRECTAS:
    output_input_dims.n = 1; output_input_dims.h = 1; output_input_dims.w = 1; output_input_dims.c = 8;
    output_filter_dims.n = 1; output_filter_dims.h = 8; output_filter_dims.w = 1; output_filter_dims.c = 1;  // 1 output, 8 inputs
    output_bias_dims.n = 1; output_bias_dims.h = 1; output_bias_dims.w = 1; output_bias_dims.c = 1;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 1;
}

int8_t xor_predict(int8_t input1, int8_t input2)
{
    int8_t input_data[2] = {input1, input2};
    
    // Capa oculta: 2 → 8 (ReLU)
    arm_fully_connected_s8(&ctx, &hidden_fc_params, &hidden_quant_params,
                          &hidden_input_dims, input_data,
                          &hidden_filter_dims, hidden_weights,
                          &hidden_bias_dims, hidden_bias,
                          &hidden_output_dims, hidden_output);
    
    // Capa salida: 8 → 1 (Tanh)
    arm_fully_connected_s8(&ctx, &output_fc_params, &output_quant_params,
                          &hidden_output_dims, hidden_output,
                          &output_filter_dims, output_weights,
                          &output_bias_dims, output_bias,
                          &output_dims, final_output);
    
    return final_output[0];
}

void test_xor_model(void) {
    printf("\n\r");
    printf("==========================================\n\r");
    printf("TEST MODELO XOR EN CMSIS-NN\n\r");
    printf("==========================================\n\r");
    
    // ✅ MOVER los arrays AQUÍ para que estén disponibles en toda la función
    int8_t test_cases[4][2] = {
        {0, 0},      // [0,0] → esperado: 0
        {0, 127},    // [0,1] → esperado: 1
        {127, 0},    // [1,0] → esperado: 1
        {127, 127}   // [1,1] → esperado: 0
    };
    
    const char* expected[4] = {"0", "1", "1", "0"};
    
    // Debug de cálculos manuales PRIMERO
    debug_simple_calculation();
    
    // Luego el debug de parámetros
    debug_quantization_params();
    
    printf("Multiplicadores: Hidden=0x%X, Output=0x%X\n\r", 
           (unsigned int)HIDDEN_MULTIPLIER, (unsigned int)OUTPUT_MULTIPLIER);
    printf("Shifts: Hidden=%d, Output=%d\n\r", HIDDEN_SHIFT, OUTPUT_SHIFT);
    
    printf("\n\r--- DEBUG DETALLADO ---\n\r");
    debug_internal_activations(0, 127);
    
    printf("\n\r--- TEST CMSIS-NN ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t result = xor_predict(test_cases[i][0], test_cases[i][1]);
        int8_t binary_result = (result > 0) ? 1 : 0;
        
        printf("CMSIS-NN - Input: [%3d, %3d] → Output: %4d → Binario: %d (esperado: %s) %s\n\r",
               test_cases[i][0], test_cases[i][1], 
               result, binary_result, expected[i],
               (binary_result == atoi(expected[i])) ? "✓" : "✗");
    }

    // ✅ TEST MANUAL 
    printf("\n\r--- TEST MANUAL (sin CMSIS-NN) ---\n\r");
    for(int i = 0; i < 4; i++) {
        int8_t result = manual_xor_predict(test_cases[i][0], test_cases[i][1]);
        int8_t binary_result = (result > 0) ? 1 : 0;
        
        printf("MANUAL - Input: [%3d, %3d] → Output: %4d → Binario: %d (esperado: %s) %s\n\r",
               test_cases[i][0], test_cases[i][1], 
               result, binary_result, expected[i],
               (binary_result == atoi(expected[i])) ? "✓" : "✗");
    }
    
    printf("\n\r==========================================\n\r");
}

void debug_internal_activations(int8_t input1, int8_t input2) {
    printf("\n\r=== DEBUG para Input [%d, %d] ===\n\r", input1, input2);
    
    int8_t input_data[2] = {input1, input2};
    int8_t hidden_output[8] = {0};
    int8_t final_output[1] = {0};
    
    // 1. Verificar entrada
    printf("Entrada: [%d, %d]\n\r", input_data[0], input_data[1]);
    
    // 2. Ejecutar solo capa oculta
    int16_t status_hidden = arm_fully_connected_s8(&ctx, &hidden_fc_params, &hidden_quant_params,
                                                  &hidden_input_dims, input_data,
                                                  &hidden_filter_dims, hidden_weights,
                                                  &hidden_bias_dims, hidden_bias,
                                                  &hidden_output_dims, hidden_output);
    
    printf("Status capa oculta: %d\n\r", status_hidden);
    
    // 3. Debug detallado de la capa de salida
    debug_output_layer(hidden_output);
    
    // 4. Ejecutar capa salida
    int16_t status_output = arm_fully_connected_s8(&ctx, &output_fc_params, &output_quant_params,
                                                  &hidden_output_dims, hidden_output,
                                                  &output_filter_dims, output_weights,
                                                  &output_bias_dims, output_bias,
                                                  &output_dims, final_output);
    
    printf("Status capa salida: %d\n\r", status_output);
    printf("Salida final: %d\n\r", final_output[0]);
    printf("==============================\n\r");
}

void debug_output_layer(int8_t* hidden_activations) {
    printf("\n\r=== DEBUG CAPA SALIDA ===\n\r");
    printf("Activaciones ocultas: ");
    for(int i = 0; i < 8; i++) {
        printf("%4d ", hidden_activations[i]);
    }
    printf("\n\r");
    
    printf("Pesos salida: ");
    for(int i = 0; i < 8; i++) {
        printf("%4d ", output_weights[i]);
    }
    printf("\n\r");
    
    // Calcular producto punto manualmente
    int32_t dot_product = 0;
    for(int i = 0; i < 8; i++) {
        dot_product += (int32_t)hidden_activations[i] * (int32_t)output_weights[i];
    }
    dot_product += output_bias[0];
    
    printf("Producto punto + bias: %ld\n\r", (long)dot_product);
    
    // CÁLCULO CORRECTO de cuantización
    int64_t multiplied = (int64_t)dot_product * output_quant_params.multiplier;
    printf("Multiplied (raw): %ld\n\r", (long)multiplied);
    
    // Aplicar shift CORRECTAMENTE
    int32_t result;
    if (output_quant_params.shift >= 0) {
        result = (int32_t)(multiplied >> (31 + output_quant_params.shift));
    } else {
        result = (int32_t)(multiplied >> (31 - output_quant_params.shift));
    }
    
    int8_t final = (int8_t)CLAMP(result, output_fc_params.activation.min, output_fc_params.activation.max);
    
    printf("Después shift: %ld, final=%d\n\r", (long)result, final);
    printf("Multiplier: 0x%lX, Shift: %ld\n\r", 
           (unsigned long)output_quant_params.multiplier, 
           (long)output_quant_params.shift);
    printf("========================\n\r");
}

void debug_quantization_params(void) {
    printf("\n\r=== DEBUG PARÁMETROS CUANTIZACIÓN ===\n\r");
    printf("Output multiplier: 0x%lX\n\r", (unsigned long)output_quant_params.multiplier);
    printf("Output shift: %ld\n\r", (long)output_quant_params.shift);
    printf("Output activation min: %ld\n\r", (long)output_fc_params.activation.min);
    printf("Output activation max: %ld\n\r", (long)output_fc_params.activation.max);
    
    // Verificar valores extremos
    printf("\n\rVerificación con valores manuales:\n\r");
    
    // Probar con un valor positivo pequeño
    int32_t test_input = 1000;
    
    printf("Test input: %ld\n\r", (long)test_input);
    printf("Multiplier: 0x%lX\n\r", (unsigned long)output_quant_params.multiplier);
    printf("Shift: %ld\n\r", (long)output_quant_params.shift);
    
    printf("==============================\n\r");
}

void debug_simple_calculation(void) {
    printf("\n\r=== CÁLCULO MANUAL SIMPLE ===\n\r");
    
    // Usar solo 2 neuronas para simplificar
    int8_t simple_hidden[2] = {127, 106};  // Dos activaciones típicas
    int8_t simple_weights[2] = {121, 119}; // Dos pesos típicos
    
    int32_t manual_dot = 0;
    for(int i = 0; i < 2; i++) {
        manual_dot += (int32_t)simple_hidden[i] * (int32_t)simple_weights[i];
    }
    manual_dot += 0;  // Sin bias por ahora
    
    printf("Cálculo manual: 127*121 + 106*119 = %ld\n\r", (long)manual_dot);
    printf("Esto debería ser POSITIVO → Si sale negativo, problema de overflow\n\r");
    printf("==============================\n\r");
}

int8_t manual_xor_predict(int8_t input1, int8_t input2) {
    int8_t input_data[2] = {input1, input2};
    int8_t hidden_output[8] = {0};
    int8_t final_output = 0;
    
    // 1. Capa oculta MANUAL (simplificada)
    for(int neuron = 0; neuron < 8; neuron++) {
        int32_t sum = 0;
        for(int input = 0; input < 2; input++) {
            sum += (int32_t)input_data[input] * (int32_t)hidden_weights[neuron*2 + input];
        }
        sum += hidden_bias[neuron];
        // Aplicar ReLU manual
        hidden_output[neuron] = (sum > 0) ? (sum >> 8) : 0; // Aproximación
    }
    
    // 2. Capa salida MANUAL
    int32_t output_sum = 0;
    for(int i = 0; i < 8; i++) {
        output_sum += (int32_t)hidden_output[i] * (int32_t)output_weights[i];
    }
    output_sum += output_bias[0];
    
    // Conversión simple sin cuantización compleja
    final_output = (output_sum >> 10); // Ajuste empírico
    
    return final_output;
}