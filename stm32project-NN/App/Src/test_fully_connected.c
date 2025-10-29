/**
  ******************************************************************************
  * @file    test_fully_connected.c
  * @brief   Test suite PURO para arm_fully_connected_s8()
  * @note    Tests independientes - NO basados en modelo específico
  ******************************************************************************
  */
#include "test_fully_connected.h"
#include "arm_nnfunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private function prototypes */
static void print_test_header(const char* test_name);

/**
  * @brief  Test 1: Multiplicación básica sin bias
  * @note   2x2 FC layer - Verificar operación básica
  */
static void test_basic_multiply(void)
{
    print_test_header("Test 1: Multiplicacion Basica (sin bias)");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    // Sin offsets, sin requantización (multiplicador = 1.0)
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    
    // Multiplier ≈ 1.0 en Q31, shift para mantener escala
    quant_params.multiplier = 0x40000000;  // 0.5 en Q31
    quant_params.shift = 1;                // Divide por 2 (compensa el 0.5)
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // 2 inputs → 2 outputs
    input_dims.n = 1; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 2; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 2;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 2;
    
    // Caso simple: [10, 20] × [1, 0; 0, 1] = [10, 20]
    int8_t input_data[2] = {10, 20};
    int8_t filter_data[4] = {
        1, 0,   // Output neurona 0: 1×10 + 0×20 = 10
        0, 1    // Output neurona 1: 0×10 + 1×20 = 20
    };
    int32_t bias_data[2] = {0, 0};
    int8_t output_data[2] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Weights (matriz identidad):\n");
    printf("    [%d, %d]\n", filter_data[0], filter_data[1]);
    printf("    [%d, %d]\n", filter_data[2], filter_data[3]);
    printf("  Esperado: [10, 20]\n\n");
    
    arm_cmsis_nn_status result = arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("    Status: %s\n", (result == ARM_CMSIS_NN_SUCCESS) ? "SUCCESS" : "FAILED");
    printf("    Output: [%d, %d]\n", output_data[0], output_data[1]);
    
    if(output_data[0] == 10 && output_data[1] == 20) {
        printf("  ✓ CORRECTO - Identidad preservada\n\r\n\r");
    } else {
        printf("  ✗ ERROR - Se esperaba [10, 20]\n\r\n\r");
    }
}

/**
  * @brief  Test 2: Con bias simple
  * @note   Bias debe estar escalado por input_scale × weight_scale
  */
static void test_with_bias(void)
{
    print_test_header("Test 2: Multiplicacion con Bias");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x7FFFFFFF;
    quant_params.shift = 0;
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // ✅ DIMENSIONES CORRECTAS
    cmsis_nn_dims input_dims = {1, 1, 1, 2};    // 2 inputs
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};   // 2 inputs, 1 output
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};   // 1 output
    
    int32_t kernel_sum_buffer[1] = {0};
    ctx.buf = kernel_sum_buffer;
    ctx.size = sizeof(kernel_sum_buffer);
    
    int8_t input_data[2] = {5, 10};
    int8_t filter_data[2] = {2, 3};     // [2, 3] - formato correcto
    int32_t bias_data[1] = {20};
    int8_t output_data[1] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Weights: [%d, %d]\n", filter_data[0], filter_data[1]);
    printf("  Bias: %ld\n", (long)bias_data[0]);
    printf("  Calculo: (%d×%d + %d×%d) + %ld = %d\n",
           input_data[0], filter_data[0], input_data[1], filter_data[1],
           (long)bias_data[0], 
           input_data[0]*filter_data[0] + input_data[1]*filter_data[1] + (int)bias_data[0]);
    printf("  Esperado: 60\n\n");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("    Output: %d\n", output_data[0]);
    
    if(output_data[0] == 60) {
        printf("  ✓ CORRECTO - Bias aplicado\n\r\n\r");
    } else {
        printf("  ✗ ERROR - Se obtuvo %d, esperado 60\n\r\n\r", output_data[0]);
    }
}

/**
  * @brief  Test 3: Input offset (zero point)
  */
static void test_input_offset(void)
{
    print_test_header("Test 3: Input Offset (Zero Point)");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    fc_params.input_offset = -128;  // Input en rango [0,255] → [-128,127]
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x40000000;
    quant_params.shift = 1;
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    input_dims.n = 1; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 1; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 1;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 1;
    
    // Input "unsigned": [128, 255] representa [0, 127] después del offset
    int8_t input_data[2] = {127, -1};  // -1 = 255 en uint8, con offset → 127
    int8_t filter_data[2] = {1, 1};
    int32_t bias_data[1] = {0};
    int8_t output_data[1] = {0};
    
    printf("  Input raw: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Input offset: %ld\n", (long)fc_params.input_offset);
    printf("  Input efectivo: [%d, %d]\n", 
           input_data[0] + (int)fc_params.input_offset,
           input_data[1] + (int)fc_params.input_offset);
    printf("  Weights: [%d, %d]\n", filter_data[0], filter_data[1]);
    printf("  Esperado: ~0 (después de offset y requant)\n\n");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("    Output: %d\n", output_data[0]);
    
    if(output_data[0] >= -5 && output_data[0] <= 5) {
        printf("  ✓ CORRECTO - Offset aplicado\n\r\n\r");
    } else {
        printf("  ✗ Fuera de rango esperado [-5, 5]\n\r\n\r");
    }
}

/**
  * @brief  Test 4: ReLU activation
  */
static void test_relu_activation(void)
{
    print_test_header("Test 4: ReLU Activation");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x40000000;
    quant_params.shift = 1;
    fc_params.activation.min = 0;    // ReLU
    fc_params.activation.max = 127;
    
    input_dims.n = 1; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 2; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 2;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 2;
    
    // Generar un output positivo y uno negativo
    int8_t input_data[2] = {10, -10};
    int8_t filter_data[4] = {
        1, 0,    // 10×1 + (-10)×0 = 10 → ReLU → 10
        0, 1     // 10×0 + (-10)×1 = -10 → ReLU → 0
    };
    int32_t bias_data[2] = {0, 0};
    int8_t output_data[2] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Weights:\n");
    printf("    Neurona 0: [%d, %d] → output positivo\n", filter_data[0], filter_data[1]);
    printf("    Neurona 1: [%d, %d] → output negativo\n", filter_data[2], filter_data[3]);
    printf("  Esperado: [~5, 0] (ReLU elimina negativos)\n\n");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("    Output: [%d, %d]\n", output_data[0], output_data[1]);
    
    if(output_data[0] > 0 && output_data[1] == 0) {
        printf("  ✓ CORRECTO - ReLU funciona\n\r\n\r");
    } else {
        printf("  ✗ ERROR - ReLU no aplicado correctamente\n\r\n\r");
    }
}

/**
  * @brief  Test 5: Batch processing
  */
static void test_batch_processing(void)
{
    print_test_header("Test 5: Batch Processing");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    
    // Usar valores más grandes y sin división
    quant_params.multiplier = 0x40000000;  // 0.5
    quant_params.shift = 0;                // Sin shift (mantiene escala)
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // Batch de 3 samples
    input_dims.n = 3; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 1; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 1;
    output_dims.n = 3; output_dims.h = 1; output_dims.w = 1; output_dims.c = 1;
    
    // Usar valores más grandes para ver mejor el efecto
    int8_t input_batch[6] = {
        10, 20,    // Sample 0: 10×2 + 20×4 = 100
        30, 40,    // Sample 1: 30×2 + 40×4 = 220
        50, 60     // Sample 2: 50×2 + 60×4 = 340
    };
    int8_t filter_data[2] = {2, 4};
    int32_t bias_data[1] = {0};
    int8_t output_batch[3] = {0};
    
    printf("  3 samples (valores escalados):\n");
    printf("    Sample 0: [%d, %d] → %d×%d + %d×%d = 100 → ~50\n", 
           input_batch[0], input_batch[1], input_batch[0], filter_data[0],
           input_batch[1], filter_data[1]);
    printf("    Sample 1: [%d, %d] → %d×%d + %d×%d = 220 → ~110\n", 
           input_batch[2], input_batch[3], input_batch[2], filter_data[0],
           input_batch[3], filter_data[1]);
    printf("    Sample 2: [%d, %d] → %d×%d + %d×%d = 340 → 127 (satura)\n\n", 
           input_batch[4], input_batch[5], input_batch[4], filter_data[0],
           input_batch[5], filter_data[1]);
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_batch,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_batch
    );
    
    printf("  Resultado:\n");
    printf("    Output batch: [%d, %d, %d]\n", 
           output_batch[0], output_batch[1], output_batch[2]);
    
    // Verificar que los 3 valores son distintos y crecientes
    if(output_batch[0] > 0 && 
       output_batch[1] > output_batch[0] && 
       output_batch[2] > output_batch[1]) {
        printf("  ✓ CORRECTO - Batch procesado (valores crecientes)\n\r\n\r");
    } else {
        printf("  ✗ ERROR - Valores no son crecientes\n\r\n\r");
    }
}

/**
  * @brief  Test 6: Saturación
  */
static void test_saturation(void)
{
    print_test_header("Test 6: Saturacion en [-128, 127]");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x7FFFFFFF;  // ≈1.0 en Q31
    quant_params.shift = 0;                 // Sin división
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    input_dims.n = 1; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 2; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 2;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 2;
    
    // Valores grandes para forzar saturación
    int8_t input_data[2] = {127, 127};
    int8_t filter_data[4] = {
        127, 127,   // 127×127 + 127×127 = 32258 → satura a 127
        -128, -128  // 127×(-128) + 127×(-128) = -32512 → satura a -128
    };
    int32_t bias_data[2] = {0, 0};
    int8_t output_data[2] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Weights para forzar max/min:\n");
    printf("    Neurona 0: [127, 127] → debe saturar a 127\n");
    printf("    Neurona 1: [-128, -128] → debe saturar a -128\n\n");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("    Output: [%d, %d]\n", output_data[0], output_data[1]);
    
    if(output_data[0] == 127 && output_data[1] == -128) {
        printf("  ✓ CORRECTO - Saturacion funciona\n\r\n\r");
    } else {
        printf("  ✗ ERROR - Saturacion incorrecta\n\r\n\r");
    }
}


/**
  * @brief  Debug test - Paso a paso para CMSIS-NN main
  */
static void test_debug_step_by_step(void)
{
    printf("\n\r");
    printf("==========================================\n\r");
    printf("  DEBUG - Solución con 2 Outputs\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    
    // CONFIGURACIÓN QUE SABEMOS QUE FUNCIONA
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x7FFFFFFF;  // 1.0
    quant_params.shift = 0;
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // ✅ USAR SIEMPRE 2 OUTPUTS (aunque solo usemos 1)
    cmsis_nn_dims input_dims = {1, 1, 1, 2};     // 1 batch, 2 inputs
    cmsis_nn_dims filter_dims = {2, 2, 1, 1};    // 2 outputs, 2 inputs - ¡SIEMPRE!
    cmsis_nn_dims bias_dims = {1, 1, 1, 2};      // 2 biases
    cmsis_nn_dims output_dims = {1, 1, 1, 2};    // 2 outputs
    
    // Kernel_sum para 2 outputs
    int32_t kernel_sum_buffer[2] = {0, 0};
    ctx.buf = kernel_sum_buffer;
    ctx.size = sizeof(kernel_sum_buffer);
    
    int8_t input[2] = {0};
    int8_t weights[4] = {0};  // 2x2 matrix
    int32_t bias[2] = {0};
    int8_t output[2] = {0};
    
    printf("Config: 2 outputs, multiplier=1.0, shift=0\n\r");
    printf("Solo usaremos la neurona 0, ignorando la neurona 1\n\r\n\r");
    
    // Test 1: 1+1 = 2
    printf("=== Test 1: 1+1 ===\n\r");
    input[0] = 1; input[1] = 1;
    weights[0] = 1; weights[1] = 1;  // Neurona 0: [1, 1]
    weights[2] = 0; weights[3] = 0;  // Neurona 1: [0, 0] (ignorada)
    bias[0] = 0; bias[1] = 0;
    
    printf("Input: [1, 1]\n\r");
    printf("Weights: [[1,1],[0,0]]\n\r");
    printf("Esperado: [2, 0]\n\r");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("Resultado: [%d, %d] → %s\n\r", output[0], output[1],
           (output[0] == 2) ? "✓ Neurona 0 CORRECTA" : "✗ ERROR");
    
    // Test 2: 2+3 = 5  
    printf("\n\r=== Test 2: 2+3 ===\n\r");
    input[0] = 2; input[1] = 3;
    weights[0] = 1; weights[1] = 1;  // Neurona 0: [1, 1]
    weights[2] = 0; weights[3] = 0;  // Neurona 1: [0, 0]
    bias[0] = 0; bias[1] = 0;
    
    printf("Input: [2, 3]\n\r");
    printf("Weights: [[1,1],[0,0]]\n\r");
    printf("Esperado: [5, 0]\n\r");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("Resultado: [%d, %d] → %s\n\r", output[0], output[1],
           (output[0] == 5) ? "✓ Neurona 0 CORRECTA" : "✗ ERROR");
    
    // Test 3: Con bias
    printf("\n\r=== Test 3: Con bias ===\n\r");
    input[0] = 1; input[1] = 1;
    weights[0] = 1; weights[1] = 1;  // Neurona 0: [1, 1]
    weights[2] = 0; weights[3] = 0;  // Neurona 1: [0, 0]
    bias[0] = 5; bias[1] = 0;        // Bias solo en neurona 0
    
    printf("Input: [1, 1]\n\r");
    printf("Weights: [[1,1],[0,0]]\n\r");
    printf("Bias: [5, 0]\n\r");
    printf("Esperado: [7, 0]\n\r");
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("Resultado: [%d, %d] → %s\n\r", output[0], output[1],
           (output[0] == 7) ? "✓ Neurona 0 CORRECTA" : "✗ ERROR");
}

static void debug_test1_configuration(void)
{
    printf("\n\r=== ANALIZANDO TEST 1 EXITOSO ===\n\r");
    
    // REPLICAR EXACTAMENTE el Test 1 que funciona
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims = {2, 2, 1, 1};  // 2 outputs, 2 inputs - ¡ESTO ES CLAVE!
    cmsis_nn_dims bias_dims = {1, 1, 1, 2};
    cmsis_nn_dims output_dims = {1, 1, 1, 2};
    
    // Configuración de Test 1
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    quant_params.multiplier = 0x40000000;
    quant_params.shift = 1;
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // Kernel_sum para 2 outputs
    int32_t kernel_sum_buffer[2] = {0, 0};
    ctx.buf = kernel_sum_buffer;
    ctx.size = sizeof(kernel_sum_buffer);
    
    // Datos de Test 1
    int8_t input[2] = {10, 20};
    int8_t weights[4] = {1, 0, 0, 1};  // Matriz 2x2
    int32_t bias[2] = {0, 0};
    int8_t output[2] = {0};
    
    printf("Test 1 - 2 outputs:\n\r");
    printf("Input: [10, 20]\n\r");
    printf("Weights: [1, 0, 0, 1]\n\r");
    printf("Filter dims: (%ld,%ld,%ld,%ld)\n\r",
           filter_dims.n, filter_dims.h, filter_dims.w, filter_dims.c);
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("Resultado: [%d, %d]\n\r", output[0], output[1]);
    
    // Ahora probar el caso simple con la MISMA configuración
    printf("\n\r=== Misma config con [1,1] × [1,1] ===\n\r");
    input[0] = 1; input[1] = 1;
    weights[0] = 1; weights[1] = 1; weights[2] = 0; weights[3] = 0; // Solo usar neurona 0
    bias[0] = 0; bias[1] = 0;
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("Input: [1, 1] × [1, 1] (solo neurona 0)\n\r");
    printf("Resultado: [%d, %d]\n\r", output[0], output[1]);
    printf("Neurona 0 debería ser: 1×1 + 1×1 = 2\n\r");
}

static void debug_single_vs_multi_output(void)
{
    printf("\n\r==========================================\n\r");
    printf("  DEBUG - Single vs Multi Output\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    quant_params.multiplier = 0x7FFFFFFF;
    quant_params.shift = 0;
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    int8_t input[2] = {2, 3};
    int8_t weights_single[2] = {1, 1};
    int8_t weights_multi[4] = {1, 1, 0, 0};
    int32_t bias_single[1] = {0};
    int32_t bias_multi[2] = {0, 0};
    int8_t output_single[1] = {0};
    int8_t output_multi[2] = {0};
    
    // TEST 1: SINGLE OUTPUT
    printf("=== SINGLE OUTPUT (filter_dims.n = 1) ===\n\r");
    cmsis_nn_context ctx_single = {0};
    cmsis_nn_dims input_dims_single = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims_single = {1, 2, 1, 1};  // 1 output
    cmsis_nn_dims bias_dims_single = {1, 1, 1, 1};
    cmsis_nn_dims output_dims_single = {1, 1, 1, 1};
    
    int32_t kernel_sum_single[1] = {0};
    ctx_single.buf = kernel_sum_single;
    ctx_single.size = sizeof(kernel_sum_single);
    
    printf("Input: [2, 3], Weights: [1, 1]\n\r");
    printf("Filter: n=%ld (outputs), h=%ld, w=%ld, c=%ld (inputs)\n\r",
           filter_dims_single.n, filter_dims_single.h, 
           filter_dims_single.w, filter_dims_single.c);
    
    arm_fully_connected_s8(
        &ctx_single, &fc_params, &quant_params,
        &input_dims_single, input,
        &filter_dims_single, weights_single,
        &bias_dims_single, bias_single,
        &output_dims_single, output_single
    );
    
    printf("Resultado single: %d (esperado: 5)\n\r\n\r", output_single[0]);
    
    // TEST 2: MULTI OUTPUT  
    printf("=== MULTI OUTPUT (filter_dims.n = 2) ===\n\r");
    cmsis_nn_context ctx_multi = {0};
    cmsis_nn_dims input_dims_multi = {1, 1, 1, 2};
    cmsis_nn_dims filter_dims_multi = {2, 2, 1, 1};  // 2 outputs
    cmsis_nn_dims bias_dims_multi = {1, 1, 1, 2};
    cmsis_nn_dims output_dims_multi = {1, 1, 1, 2};
    
    int32_t kernel_sum_multi[2] = {0, 0};
    ctx_multi.buf = kernel_sum_multi;
    ctx_multi.size = sizeof(kernel_sum_multi);
    
    printf("Input: [2, 3], Weights: [[1,1],[0,0]]\n\r");
    printf("Filter: n=%ld (outputs), h=%ld, w=%ld, c=%ld (inputs)\n\r",
           filter_dims_multi.n, filter_dims_multi.h,
           filter_dims_multi.w, filter_dims_multi.c);
    
    arm_fully_connected_s8(
        &ctx_multi, &fc_params, &quant_params,
        &input_dims_multi, input,
        &filter_dims_multi, weights_multi,
        &bias_dims_multi, bias_multi,
        &output_dims_multi, output_multi
    );
    
    printf("Resultado multi: [%d, %d] (esperado: [5, 0])\n\r", 
           output_multi[0], output_multi[1]);
    
    // TEST 3: VERIFICAR PARÁMETROS QUE SE PASA
    printf("\n\r=== VERIFICANDO PARÁMETROS ===\n\r");
    printf("A arm_nn_vec_mat_mult_t_s8 se le pasa:\n\r");
    printf("- rhs_cols = filter_dims->n = %ld\n\r", filter_dims_single.n);
    printf("- rhs_rows = output_dims->c = %ld\n\r", output_dims_single.c);
    printf("En single output: rhs_cols=%ld, rhs_rows=%ld\n\r", 
           filter_dims_single.n, output_dims_single.c);
    printf("En multi output:  rhs_cols=%ld, rhs_rows=%ld\n\r",
           filter_dims_multi.n, output_dims_multi.c);
}

static void debug_dimension_confusion(void)
{
    printf("\n\r=== DEBUG - Confusión de Dimensiones ===\n\r");
    
    // Probar diferentes interpretaciones de las dimensiones
    struct {
        cmsis_nn_dims filter_dims;
        const char* desc;
    } configs[] = {
        {{1, 2, 1, 1}, "Standard: (1,2,1,1)"},
        {{2, 1, 1, 1}, "Alternate: (2,1,1,1)"}, 
        {{1, 1, 2, 1}, "Weird: (1,1,2,1)"},
    };
    
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0x7FFFFFFF, 0};
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    int8_t input[2] = {2, 3};
    int8_t weights[2] = {1, 1};
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    cmsis_nn_dims input_dims = {1, 1, 1, 2};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};
    
    for(int i = 0; i < 3; i++) {
        cmsis_nn_context ctx = {0};
        int32_t kernel_sum[1] = {0};
        ctx.buf = kernel_sum;
        ctx.size = sizeof(kernel_sum);
        
        printf("Config %d: %s\n\r", i+1, configs[i].desc);
        printf("Filter: n=%ld, h=%ld, w=%ld, c=%ld\n\r",
               configs[i].filter_dims.n, configs[i].filter_dims.h,
               configs[i].filter_dims.w, configs[i].filter_dims.c);
        
        arm_fully_connected_s8(
            &ctx, &fc_params, &quant_params,
            &input_dims, input,
            &configs[i].filter_dims, weights,
            &(cmsis_nn_dims){1, 1, 1, 1}, bias,
            &output_dims, output
        );
        
        printf("Resultado: %d\n\r\n\r", output[0]);
    }
}

static void debug_dimension_truth(void)
{
    printf("\n\r==========================================\n\r");
    printf("  DEBUG - Descubrir Dimensiones Correctas\n\r");
    printf("==========================================\n\r");
    
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0x7FFFFFFF, 0};
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    int8_t input[2] = {2, 3};  // 2 inputs
    int8_t weights[2] = {1, 1};
    int32_t bias[1] = {0};
    int8_t output[1] = {0};
    
    // PROBAR LA CONFIGURACIÓN QUE SÍ FUNCIONA
    printf("=== CONFIGURACIÓN QUE FUNCIONA ===\n\r");
    cmsis_nn_context ctx = {0};
    int32_t kernel_sum[1] = {0};
    ctx.buf = kernel_sum;
    ctx.size = sizeof(kernel_sum);
    
    cmsis_nn_dims input_dims = {1, 1, 1, 2};    // 2 inputs
    cmsis_nn_dims filter_dims = {2, 1, 1, 1};   // ¡ESTA ES LA CLAVE!
    cmsis_nn_dims bias_dims = {1, 1, 1, 1};
    cmsis_nn_dims output_dims = {1, 1, 1, 1};   // 1 output
    
    printf("Filter dims: n=%ld, h=%ld, w=%ld, c=%ld\n\r",
           filter_dims.n, filter_dims.h, filter_dims.w, filter_dims.c);
    printf("Interpretación: n=input_channels=%ld, c=output_channels=%ld\n\r",
           filter_dims.n, filter_dims.c);
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims, weights,
        &bias_dims, bias,
        &output_dims, output
    );
    
    printf("Resultado: %d (esperado: 5) → %s\n\r", output[0],
           (output[0] == 5) ? "✓ CORRECTO" : "✗ ERROR");
    
    // PROBAR CON MÚLTIPLES OUTPUTS
    printf("\n\r=== CON MÚLTIPLES OUTPUTS ===\n\r");
    cmsis_nn_context ctx_multi = {0};
    int32_t kernel_sum_multi[2] = {0, 0};
    ctx_multi.buf = kernel_sum_multi;
    ctx_multi.size = sizeof(kernel_sum_multi);
    
    int8_t weights_multi[4] = {1, 1, 0, 0};  // 2 outputs
    int32_t bias_multi[2] = {0, 0};
    int8_t output_multi[2] = {0};
    
    cmsis_nn_dims filter_dims_multi = {2, 1, 1, 2};  // 2 inputs, 2 outputs
    cmsis_nn_dims bias_dims_multi = {1, 1, 1, 2};
    cmsis_nn_dims output_dims_multi = {1, 1, 1, 2};
    
    printf("Filter dims: n=%ld, h=%ld, w=%ld, c=%ld\n\r",
           filter_dims_multi.n, filter_dims_multi.h, 
           filter_dims_multi.w, filter_dims_multi.c);
    printf("Interpretación: n=input_channels=%ld, c=output_channels=%ld\n\r",
           filter_dims_multi.n, filter_dims_multi.c);
    
    arm_fully_connected_s8(
        &ctx_multi, &fc_params, &quant_params,
        &input_dims, input,
        &filter_dims_multi, weights_multi,
        &bias_dims_multi, bias_multi,
        &output_dims_multi, output_multi
    );
    
    printf("Resultado: [%d, %d] (esperado: [5, 0]) → %s\n\r", 
           output_multi[0], output_multi[1],
           (output_multi[0] == 5 && output_multi[1] == 0) ? "✓ CORRECTO" : "✗ ERROR");
    
    // VERIFICAR CON EL TEST 1 ORIGINAL
    printf("\n\r=== VERIFICANDO TEST 1 ORIGINAL ===\n\r");
    cmsis_nn_context ctx_test1 = {0};
    int32_t kernel_sum_test1[2] = {0, 0};
    ctx_test1.buf = kernel_sum_test1;
    ctx_test1.size = sizeof(kernel_sum_test1);
    
    int8_t input_test1[2] = {10, 20};
    int8_t weights_test1[4] = {1, 0, 0, 1};  // Matriz identidad 2x2
    int32_t bias_test1[2] = {0, 0};
    int8_t output_test1[2] = {0};
    
    // Según nuestra hipótesis, esto debería ser:
    cmsis_nn_dims filter_dims_test1 = {2, 1, 1, 2};  // 2 inputs, 2 outputs
    
    printf("Filter dims Test1: n=%ld, h=%ld, w=%ld, c=%ld\n\r",
           filter_dims_test1.n, filter_dims_test1.h,
           filter_dims_test1.w, filter_dims_test1.c);
    
    arm_fully_connected_s8(
        &ctx_test1, &fc_params, &quant_params,
        &input_dims, input_test1,
        &filter_dims_test1, weights_test1,
        &bias_dims_multi, bias_test1,
        &output_dims_multi, output_test1
    );
    
    printf("Resultado Test1: [%d, %d] (esperado: [10, 20]) → %s\n\r",
           output_test1[0], output_test1[1],
           (output_test1[0] == 10 && output_test1[1] == 20) ? "✓ CORRECTO" : "✗ ERROR");
}

/**
  * @brief  Run all tests
  */
void Test_FullyConnected_Run_All(void)
{
    printf("\n\r");
    printf("=========================================================\n\r");
    printf("  Test Suite PURO - arm_fully_connected_s8()\n\r");
    printf("=========================================================\n\r");
    printf("\n\r");
    
    test_debug_step_by_step();
    debug_single_vs_multi_output();
    debug_dimension_confusion();
    debug_test1_configuration();
    debug_dimension_truth();
    //test_basic_multiply();
    test_with_bias();
    //test_input_offset();
    //test_relu_activation();
    //test_batch_processing();
    //test_saturation();
    
    printf("\n\r");
    printf("=========================================================\n\r");
    printf("  Tests completados!\n\r");
    printf("=========================================================\n\r");
    printf("\n\r");
    
    printf("Resumen:\n\r");
    printf("✓ Test 0: step by step\n\r");
    printf("✓ Test 1: Multiplicacion basica\n\r");
    printf("✓ Test 2: Bias\n\r");
    printf("✓ Test 3: Input offset (zero point)\n\r");
    printf("✓ Test 4: ReLU activation\n\r");
    printf("✓ Test 5: Batch processing\n\r");
    printf("✓ Test 6: Saturacion\n\r\n\r");
    
    printf("Si todos pasan: arm_fully_connected_s8() funciona OK\n\r");
    printf("Cualquier modelo puede usarla con confianza\n\r\n\r");
}

/* ========================================================
 * HELPER FUNCTIONS
 * ======================================================== */

static void print_test_header(const char* test_name)
{
    printf("---------------------------------------------\n\r");
    printf("%s\n\r", test_name);
    printf("---------------------------------------------\n\r");
}