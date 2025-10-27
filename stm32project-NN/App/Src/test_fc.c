/**
  ******************************************************************************
  * @file    test_fc.c
  * @brief   Test suite for arm_fully_connected_s8() - Educational examples
  ******************************************************************************
  */
#include "test_fc.h"
#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"
#include <stdio.h>
#include <string.h>

/* Private function prototypes */
static void print_test_header(const char* test_name);
static void print_vector_q7(const char* name, const int8_t* vec, uint32_t len);

/**
  * @brief  Test 1: Red MÍNIMA 2→1 (la más simple posible)
  *         Input: 2 valores → Output: 1 valor
  */
static void test_minimal_network(void)
{
    print_test_header("Test 1: Red Mínima 2→1");
    
    // ============================================================
    // DEFINIR RED: 2 inputs → 1 output (sin capa oculta)
    // ============================================================
    
    // Input: [1.0, 0.0] en Q7
    int8_t input[2] = {127, 0};
    
    // Weights: [0.5, 0.5] en Q7
    int8_t weights[2] = {63, 63};  // 0.5 × 127 ≈ 63
    
    // Bias: 0
    int32_t bias[1] = {0};
    
    // Output buffer
    int8_t output[1] = {0};
    
    // Buffer temporal (requerido por CMSIS-NN)
    int32_t buffer[2] = {0, 0};  // Tamaño = num_inputs
    
    printf("  Red: 2 inputs → 1 output\n\r");
    printf("  Input:   [%d, %d] → [1.0, 0.0]\n\r", input[0], input[1]);
    printf("  Weights: [%d, %d] → [0.5, 0.5]\n\r", weights[0], weights[1]);
    printf("  Bias:    %ld\n\r\n\r", bias[0]);
    
    // ============================================================
    // CONFIGURAR ESTRUCTURAS CMSIS-NN
    // ============================================================
    
    // Context: buffer temporal
    cmsis_nn_context ctx;
    ctx.buf = buffer;
    ctx.size = sizeof(buffer);
    
    // Dimensiones de entrada
    cmsis_nn_dims input_dims;
    input_dims.n = 1;  // batch size
    input_dims.h = 1;  // height (no aplica para FC)
    input_dims.w = 1;  // width (no aplica para FC)
    input_dims.c = 2;  // channels = número de inputs
    
    // Dimensiones de filtro/pesos
    cmsis_nn_dims filter_dims;
    filter_dims.n = 1;  // número de outputs
    filter_dims.h = 1;
    filter_dims.w = 1;
    filter_dims.c = 2;  // debe coincidir con input_dims.c
    
    // Dimensiones de bias
    cmsis_nn_dims bias_dims;
    bias_dims.n = 1;
    bias_dims.h = 1;
    bias_dims.w = 1;
    bias_dims.c = 1;  // número de outputs
    
    // Dimensiones de salida
    cmsis_nn_dims output_dims;
    output_dims.n = 1;
    output_dims.h = 1;
    output_dims.w = 1;
    output_dims.c = 1;  // número de outputs
    
    // Parámetros de fully connected
    cmsis_nn_fc_params fc_params;
    fc_params.input_offset = 0;    // Sin offset
    fc_params.filter_offset = 0;   // Sin offset
    fc_params.output_offset = 0;   // Sin offset
    fc_params.activation.min = -128;  // Sin activación (rango completo)
    fc_params.activation.max = 127;
    
    // Parámetros de cuantización
    cmsis_nn_per_tensor_quant_params quant_params;
    quant_params.multiplier = 2147483647;  
    quant_params.shift = -7;  // Shift para Q14 → Q7
    
    printf("  Estructuras configuradas:\n\r");
    printf("    input_dims.c:  %ld (inputs)\n\r", input_dims.c);
    printf("    output_dims.c: %ld (outputs)\n\r", output_dims.c);
    printf("    multiplier:    %ld\n\r", quant_params.multiplier);
    printf("    shift:         %ld\n\r\n\r", quant_params.shift);
    
    // ============================================================
    // LLAMAR A CMSIS-NN
    // ============================================================
    
    arm_cmsis_nn_status status = arm_fully_connected_s8(
        &ctx,
        &fc_params,
        &quant_params,
        &input_dims,
        input,
        &filter_dims,
        weights,
        &bias_dims,
        bias,
        &output_dims,
        output
    );
    
    // ============================================================
    // VERIFICAR RESULTADO
    // ============================================================
    
    printf("  Status CMSIS-NN: %s\n\r", 
           status == ARM_CMSIS_NN_SUCCESS ? "SUCCESS" : "ERROR");
    printf("  Output: %d (Q7)\n\r", output[0]);
    printf("  Output flotante: %.4f\n\r\n\r", output[0] / 127.0f);
    
    // Cálculo manual para verificar
    int32_t manual = (127 * 63 + 0 * 63) >> 7;  // Q14 → Q7
    printf("  Verificación manual:\n\r");
    printf("    (127×63 + 0×63) >> 7 = %ld\n\r", manual);
    printf("    Match: %s\n\r\n\r", manual == output[0] ? "✓" : "✗");
}

/**
  * @brief  Test 2: Red 2→2 (dos outputs)
  */
static void test_two_outputs(void)
{
    print_test_header("Test 2: Red 2→2");
    
    // Input: [1.0, 0.5] en Q7
    int8_t input[2] = {127, 63};
    
    // Weights: 2 neuronas, cada una con 2 pesos
    // Neurona 0: [1.0, 0.0] → [127, 0]
    // Neurona 1: [0.0, 1.0] → [0, 127]
    int8_t weights[4] = {
        127, 0,    // Neurona 0
        0, 127     // Neurona 1
    };
    
    // Bias: [0, 0]
    int32_t bias[2] = {0, 0};
    
    // Output buffer (2 outputs)
    int8_t output[2] = {0, 0};
    
    // Buffer temporal
    int32_t buffer[2] = {0, 0};
    
    printf("  Red: 2 inputs → 2 outputs\n\r");
    printf("  Input: [%d, %d] → [1.0, 0.5]\n\r\n\r", input[0], input[1]);
    
    // Configurar estructuras (similar a test 1)
    cmsis_nn_context ctx = {.buf = buffer, .size = sizeof(buffer)};
    
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {2, 1, 1, 2};  // 2 outputs, 2 inputs
    cmsis_nn_dims bias_dims = {1, 1, 1, 2};
    cmsis_nn_dims output_dims = {1, 1, 1, 2};
    
    cmsis_nn_fc_params fc_params = {
        .input_offset = 0,
        .filter_offset = 0,
        .output_offset = 0,
        .activation = {.min = -128, .max = 127}
    };
    
    cmsis_nn_per_tensor_quant_params quant_params = {
        .multiplier = 1073741824,
        .shift = -7
    };
    
    // Llamar CMSIS-NN
    arm_cmsis_nn_status status = arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("  Status: %s\n\r", 
           status == ARM_CMSIS_NN_SUCCESS ? "SUCCESS" : "ERROR");
    printf("  Output[0]: %d → %.4f\n\r", output[0], output[0]/127.0f);
    printf("  Output[1]: %d → %.4f\n\r\n\r", output[1], output[1]/127.0f);
    
    // Verificación manual
    int32_t manual0 = (127 * 127 + 63 * 0) >> 7;
    int32_t manual1 = (127 * 0 + 63 * 127) >> 7;
    printf("  Verificación manual:\n\r");
    printf("    Neurona 0: %ld (esperado), %d (obtenido) %s\n\r", 
           manual0, output[0], manual0 == output[0] ? "✓" : "✗");
    printf("    Neurona 1: %ld (esperado), %d (obtenido) %s\n\r\n\r", 
           manual1, output[1], manual1 == output[1] ? "✓" : "✗");
}

/**
  * @brief  Test 3: Con ReLU activation
  */
static void test_with_relu(void)
{
    print_test_header("Test 3: Con Activación ReLU");
    
    // Input que dará resultado negativo
    int8_t input[2] = {127, -127};
    
    // Weights que suman a negativo
    int8_t weights[2] = {-63, -63};
    
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    int32_t buffer[2] = {0, 0};
    
    printf("  Input: [%d, %d] → [1.0, -1.0]\n\r", input[0], input[1]);
    printf("  Weights: [%d, %d] → [-0.5, -0.5]\n\r", weights[0], weights[1]);
    printf("  Sin activación, debería dar negativo\n\r");
    printf("  Con ReLU, debería dar 0\n\r\n\r");
    
    // Configurar
    cmsis_nn_context ctx = {.buf = buffer, .size = sizeof(buffer)};
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {1, 1, 1, 2};
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    cmsis_nn_fc_params fc_params = {
        .input_offset = 0,
        .filter_offset = 0,
        .output_offset = 0,
        .activation = {.min = 0, .max = 127}  // ← ReLU: min = 0
    };
    
    cmsis_nn_per_tensor_quant_params quant_params = {
        .multiplier = 1073741824,
        .shift = -7
    };
    
    // Llamar
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("  Output con ReLU: %d\n\r", output[0]);
    printf("  Resultado: %s\n\r\n\r", 
           output[0] == 0 ? "✓ ReLU funcionó" : "✗ Error");
}

/**
  * @brief  Run all fully connected tests
  */
void Test_FC_Run_All(void)
{
    printf("\n\r");
    printf("========================================\n\r");
    printf("  CMSIS-NN arm_fully_connected_s8()\n\r");
    printf("========================================\n\r");
    printf("\n\r");
    
    test_minimal_network();
    test_two_outputs();
    test_with_relu();
    
    printf("\n\r");
    printf("========================================\n\r");
    printf("  Todos los tests completados!\n\r");
    printf("========================================\n\r");
    printf("\n\r");
    
    printf("Conclusiones:\n\r");
    printf("✓ arm_fully_connected_s8() hace: Input × Weights + Bias\n\r");
    printf("✓ Necesitas configurar 4 estructuras (ctx, fc_params, quant, dims)\n\r");
    printf("✓ input_dims.c = número de inputs\n\r");
    printf("✓ output_dims.c = número de outputs\n\r");
    printf("✓ filter_dims = [outputs, 1, 1, inputs]\n\r");
    printf("✓ activation.min/max controla ReLU y saturación\n\r");
    printf("✓ quant_params controla la escala del resultado\n\r\n\r");
}

/* ========================================================
 * HELPER FUNCTIONS
 * ======================================================== */

static void print_test_header(const char* test_name)
{
    printf("----------------------------------------\n\r");
    printf("%s\n\r", test_name);
    printf("----------------------------------------\n\r");
}

static void print_vector_q7(const char* name, const int8_t* vec, uint32_t len)
{
    printf("  %s: [", name);
    for(uint32_t i = 0; i < len; i++) {
        printf("%4d", vec[i]);
        if(i < len-1) printf(",");
    }
    printf("]\n\r");
}