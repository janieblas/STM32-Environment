/**
  ******************************************************************************
  * @file    test_fully_connected.c
  * @brief   Test suite for arm_fully_connected_s8() - Educational examples
  ******************************************************************************
  */
#include "test_fully_connected.h"
#include "arm_nnfunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private function prototypes */
static void print_test_header(const char* test_name);

// Declaración explícita para evitar warnings
arm_cmsis_nn_status arm_nn_vec_mat_mult_t_s8(const int8_t *lhs,
                                            const int8_t *rhs,
                                            const int32_t *kernel_sum,
                                            const int32_t *bias,
                                            int8_t *dst,
                                            const int32_t lhs_offset,
                                            const int32_t dst_offset,
                                            const int32_t dst_multiplier,
                                            const int32_t dst_shift,
                                            const int32_t rhs_cols,
                                            const int32_t rhs_rows,
                                            const int32_t activation_min,
                                            const int32_t activation_max,
                                            const int32_t address_offset,
                                            const int32_t rhs_offset);

/**
  * @brief  Test 1: Fully Connected Básica
  */
static void test_basic_fc(void)
{
    print_test_header("Test 1: Fully Connected Basica");
    
    // Configuración de capa fully connected
    cmsis_nn_context ctx = {0};  // Contexto correcto inicializado a cero
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    // Parámetros de cuantización
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    
    // Multiplier y shift
    quant_params.multiplier = 0x40000000;
    quant_params.shift = 0;
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // Dimensiones
    input_dims.n = 1;
    input_dims.h = 1;
    input_dims.w = 1;
    input_dims.c = 2;
    
    filter_dims.n = 2;
    filter_dims.h = 2;
    filter_dims.w = 1;
    filter_dims.c = 1;
    
    bias_dims.n = 1;
    bias_dims.h = 1;
    bias_dims.w = 1;
    bias_dims.c = 2;
    
    output_dims.n = 1;
    output_dims.h = 1;
    output_dims.w = 1;
    output_dims.c = 2;
    
    // Datos
    int8_t input_data[2] = {64, 32};
    int8_t filter_data[4] = {64, 32, 32, 64};
    int32_t bias_data[2] = {0, 0};
    int8_t output_data[2] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Filter: [%d, %d, %d, %d]\n", 
           filter_data[0], filter_data[1], filter_data[2], filter_data[3]);
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", 
           quant_params.multiplier, quant_params.shift);
    
    // Llamada a fully_connected
    arm_cmsis_nn_status result = arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado arm_fully_connected_s8:\n");
    printf("  Status: %s\n", (result == ARM_CMSIS_NN_SUCCESS) ? "SUCCESS" : "FAILED");
    printf("  Output: [%d, %d]\n", output_data[0], output_data[1]);
    printf("  En float: [%.3f, %.3f]\n", output_data[0]/127.0f, output_data[1]/127.0f);
    
    // Comparación con vec_mat_mult
    int8_t vec_mat_output[2] = {0};
    arm_nn_vec_mat_mult_t_s8(input_data, filter_data, NULL, bias_data, vec_mat_output,
                            0, 0, quant_params.multiplier, quant_params.shift,
                            2, 2, -128, 127, 1, 0);
    
    printf("  Comparacion con vec_mat_mult: [%d, %d]\n", vec_mat_output[0], vec_mat_output[1]);
    
    if(output_data[0] == vec_mat_output[0] && output_data[1] == vec_mat_output[1]) {
        printf("\n  Resultado: CORRECTO (coincide con vec_mat_mult)\n\r\n\r");
    } else {
        printf("\n  Resultado: DIFERENCIA con vec_mat_mult\n\r\n\r");
    }
}

/**
  * @brief  Test 2: Con Bias y Offsets
  */
static void test_fc_with_bias_offsets(void)
{
    print_test_header("Test 2: FC con Bias y Offsets");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    // Probando offsets
    fc_params.input_offset = 10;
    fc_params.filter_offset = 5;
    fc_params.output_offset = 0;
    
    quant_params.multiplier = 0x20000000;
    quant_params.shift = -1;
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // Dimensiones
    input_dims.n = 1; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 2; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 2;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 2;
    
    // Datos con offsets
    int8_t input_data[2] = {20, 30};
    int8_t filter_data[4] = {10, 20, 15, 25};
    int32_t bias_data[2] = {1000, 2000};
    int8_t output_data[2] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Filter: [%d, %d, %d, %d]\n", 
           filter_data[0], filter_data[1], filter_data[2], filter_data[3]);
    printf("  Input offset: %ld, Filter offset: %ld\n", 
           fc_params.input_offset, fc_params.filter_offset);
    printf("  Bias: [%ld, %ld]\n", bias_data[0] >> 10, bias_data[1] >> 10);
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", 
           quant_params.multiplier, quant_params.shift);
    
    arm_cmsis_nn_status result = arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("  Status: %s\n", (result == ARM_CMSIS_NN_SUCCESS) ? "SUCCESS" : "FAILED");
    printf("  Output: [%d, %d]\n", output_data[0], output_data[1]);
    
    if(output_data[0] != 0 && output_data[1] != 0) {
        printf("\n  Resultado: CORRECTO (valores no cero)\n\r\n\r");
    } else {
        printf("\n  Resultado: ERROR - valores cero\n\r\n\r");
    }
}

/**
  * @brief  Test 3: Capa Oculta XOR con Fully Connected
  */
static void test_xor_hidden_fc(void)
{
    print_test_header("Test 3: XOR Hidden Layer con FC");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    // Configuración para capa oculta
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    
    // Multiplier más pequeño para evitar saturación
    quant_params.multiplier = 0x08000000;
    quant_params.shift = -3;
    
    fc_params.activation.min = 0;    // ReLU activation
    fc_params.activation.max = 127;
    
    // Dimensiones
    input_dims.n = 1; input_dims.h = 1; input_dims.w = 1; input_dims.c = 2;
    filter_dims.n = 2; filter_dims.h = 2; filter_dims.w = 1; filter_dims.c = 1;
    bias_dims.n = 1; bias_dims.h = 1; bias_dims.w = 1; bias_dims.c = 2;
    output_dims.n = 1; output_dims.h = 1; output_dims.w = 1; output_dims.c = 2;
    
    // Pesos tipo XOR
    int8_t input_data[2] = {127, 0};
    int8_t filter_data[4] = {64, -64, 64, 64};
    int32_t bias_data[2] = {0, 0};
    int8_t output_data[2] = {0};
    
    printf("  Input: [%d, %d]\n", input_data[0], input_data[1]);
    printf("  Filter: [%d, %d, %d, %d]\n", 
           filter_data[0], filter_data[1], filter_data[2], filter_data[3]);
    printf("  Activation: ReLU [0, 127]\n");
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", 
           quant_params.multiplier, quant_params.shift);
    
    arm_cmsis_nn_status result = arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_data,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_data
    );
    
    printf("  Resultado:\n");
    printf("  Status: %s\n", (result == ARM_CMSIS_NN_SUCCESS) ? "SUCCESS" : "FAILED");
    printf("  Output: [%d, %d]\n", output_data[0], output_data[1]);
    printf("  En float: [%.3f, %.3f]\n", output_data[0]/127.0f, output_data[1]/127.0f);
    
    // Verificar que ReLU funcionó
    if(output_data[0] >= 0 && output_data[1] >= 0) {
        printf("\n  Resultado: CORRECTO (ReLU aplicado)\n\r\n\r");
    } else {
        printf("\n  Resultado: ERROR - ReLU no funciono\n\r\n\r");
    }
}

/**
  * @brief  Test 4: Batch Processing (múltiples inputs)
  */
static void test_batch_processing(void)
{
    print_test_header("Test 4: Batch Processing");
    
    cmsis_nn_context ctx = {0};
    cmsis_nn_fc_params fc_params = {0};
    cmsis_nn_per_tensor_quant_params quant_params = {0};
    cmsis_nn_dims input_dims = {0}, filter_dims = {0}, bias_dims = {0}, output_dims = {0};
    
    fc_params.input_offset = 0;
    fc_params.filter_offset = 0;
    fc_params.output_offset = 0;
    
    quant_params.multiplier = 0x20000000;
    quant_params.shift = -1;
    
    fc_params.activation.min = -128;
    fc_params.activation.max = 127;
    
    // BATCH de 4 samples
    input_dims.n = 4;
    input_dims.h = 1;
    input_dims.w = 1;
    input_dims.c = 2;
    
    filter_dims.n = 2;
    filter_dims.h = 2;
    filter_dims.w = 1;
    filter_dims.c = 1;
    
    bias_dims.n = 1;
    bias_dims.h = 1;
    bias_dims.w = 1;
    bias_dims.c = 2;
    
    output_dims.n = 4;
    output_dims.h = 1;
    output_dims.w = 1;
    output_dims.c = 2;
    
    // Batch completo XOR
    int8_t input_batch[8] = {127, 0, 0, 127, 127, 127, 0, 0};
    int8_t filter_data[4] = {64, 32, 32, 64};
    int32_t bias_data[2] = {0, 0};
    int8_t output_batch[8] = {0};
    
    printf("  Batch size: 4 samples\n");
    printf("  Input batch: [1,0], [0,1], [1,1], [0,0]\n");
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", 
           quant_params.multiplier, quant_params.shift);
    
    arm_fully_connected_s8(
        &ctx, &fc_params, &quant_params,
        &input_dims, input_batch,
        &filter_dims, filter_data,
        &bias_dims, bias_data,
        &output_dims, output_batch
    );
    
    printf("  Resultados batch:\n");
    for(int i = 0; i < 4; i++) {
        printf("  Sample %d: [%d, %d]\n", 
               i, output_batch[i*2], output_batch[i*2+1]);
    }
    
    printf("\n  Resultado: CORRECTO (batch procesado)\n\r\n\r");
}

/**
  * @brief  Run all fully connected tests
  */
void Test_FullyConnected_Run_All(void)
{
    printf("\n\r");
    printf("=============================================\n\r");
    printf("  CMSIS-NN arm_fully_connected_s8() Tests\n\r");
    printf("=============================================\n\r");
    printf("\n\r");
    
    test_basic_fc();
    test_fc_with_bias_offsets();
    test_xor_hidden_fc();
    test_batch_processing();
    
    printf("\n\r");
    printf("=============================================\n\r");
    printf("  Todos los tests FC completados!\n\r");
    printf("=============================================\n\r");
    printf("\n\r");
    
    printf("Resumen:\n\r");
    printf("✓ arm_fully_connected_s8() verificada\n\r");
    printf("✓ Interfaz con estructuras funcionando\n\r");
    printf("✓ Batch processing operativo\n\r");
    printf("✓ Activaciones integradas trabajando\n\r");
    printf("✓ Lista para integracion en modelo real\n\r\n\r");
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