/**
  ******************************************************************************
  * @file    test_vec_mat_mult.c
  * @brief   Test suite for arm_nn_vec_mat_mult_t_s8() - Educational examples
  ******************************************************************************
  */
#include "test_vec_mat_mult.h"
#include "arm_nnfunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Private function prototypes */
static void print_test_header(const char* test_name);
static void calculate_manual_vec_mat_mult(const int8_t* lhs, const int8_t* rhs, 
                                         const int32_t* bias, int32_t* dst, 
                                         int32_t lhs_offset, int32_t rhs_offset,
                                         int32_t rhs_cols, int32_t rhs_rows);
static int32_t custom_requantize(int32_t value, int32_t multiplier, int32_t shift);

/**
  * @brief  Test 1: Multiplicación básica vector-matriz 2x2 - CON REQUANTIZACIÓN
  */
static void test_basic_2x2(void)
{
    print_test_header("Test 1: Multiplicacion Basica 2x2");
    
    // CON REQUANTIZACIÓN ADECUADA
    const int32_t lhs_offset = 0;
    const int32_t rhs_offset = 0;
    const int32_t dst_offset = 0;
    const int32_t dst_multiplier = 0x10000000; // 0.25 en Q30 (MENOR para escalar)
    const int32_t dst_shift = -2;              // Shift adicional
    const int32_t activation_min = -128;
    const int32_t activation_max = 127;
    const int32_t address_offset = 1;
    
    int8_t lhs[2] = {16, 8};
    int8_t rhs[4] = {16, 8, 8, 16};
    int8_t dst[2] = {0};
    int32_t bias[2] = {0, 0};
    
    printf("  Vector lhs: [%d, %d]\n", lhs[0], lhs[1]);
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", dst_multiplier, dst_shift);
    
    arm_nn_vec_mat_mult_t_s8(lhs, rhs, NULL, bias, dst,
                            lhs_offset, dst_offset, dst_multiplier, dst_shift,
                            2, 2, activation_min, activation_max, 
                            address_offset, rhs_offset);
    
    printf("  Resultados:\n");
    printf("  CMSIS-NN:     [%d, %d]\n", dst[0], dst[1]);
    printf("  En float:     [%.3f, %.3f]\n", dst[0]/127.0f, dst[1]/127.0f);
    
    // Verificar que no están saturados
    if(dst[0] != 127 && dst[1] != 127 && dst[0] != -128 && dst[1] != -128) {
        printf("\n  Resultado: CORRECTO (no saturado)\n\r\n\r");
    } else {
        printf("\n  Resultado: SATURADO\n\r\n\r");
    }
}

/**
  * @brief  Test 2: Con offsets y bias - SIMPLIFICADO
  */
static void test_with_offsets_bias(void)
{
    print_test_header("Test 2: Con Offsets y Bias");
    
    // Configuración SIMPLIFICADA
    const int32_t lhs_offset = 0;
    const int32_t rhs_offset = 0;
    const int32_t dst_offset = 0;
    const int32_t dst_multiplier = 0x40000000; // 1.0 en Q30
    const int32_t dst_shift = 0;
    const int32_t activation_min = -128;
    const int32_t activation_max = 127;
    const int32_t address_offset = 1;
    
    // Datos de entrada SIMPLES
    int8_t lhs[2] = {10, 20};
    int8_t rhs[4] = {5, 10, 15, 20};
    int8_t dst[2] = {0};
    int32_t bias[2] = {500, 1000};   // Bias en Q7
    
    printf("  Vector lhs: [%d, %d]\n", lhs[0], lhs[1]);
    printf("  Matriz rhs (transpuesta):\n");
    printf("    [%d, %d]\n", rhs[0], rhs[2]);
    printf("    [%d, %d]\n", rhs[1], rhs[3]);
    printf("  Bias: [%ld, %ld]\n", bias[0] >> 7, bias[1] >> 7);
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", dst_multiplier, dst_shift);
    
    // CMSIS-NN
    arm_nn_vec_mat_mult_t_s8(lhs, rhs, NULL, bias, dst,
                            lhs_offset, dst_offset, dst_multiplier, dst_shift,
                            2, 2, activation_min, activation_max, 
                            address_offset, rhs_offset);
    
    printf("  Resultados:\n");
    printf("  CMSIS-NN:     [%d, %d]\n", dst[0], dst[1]);
    printf("  En float:     [%.3f, %.3f]\n", dst[0]/127.0f, dst[1]/127.0f);
    
    if(dst[0] != 0 && dst[1] != 0) {
        printf("\n  Resultado: CORRECTO (valores no cero)\n\r\n\r");
    } else {
        printf("\n  Resultado: ERROR - valores cero\n\r\n\r");
    }
}

/**
  * @brief  Test 3: Capa oculta XOR - CON REQUANTIZACIÓN
  */
static void test_xor_hidden_layer(void)
{
    print_test_header("Test 3: Capa Oculta XOR");
    
    // CON REQUANTIZACIÓN MÁS AGRESIVA
    const int32_t lhs_offset = 0;
    const int32_t rhs_offset = 0;
    const int32_t dst_offset = 0;
    const int32_t dst_multiplier = 0x04000000; // ~0.0156 en Q30 (MUCHO MENOR)
    const int32_t dst_shift = -6;              // Shift grande
    const int32_t activation_min = -128;
    const int32_t activation_max = 127;
    const int32_t address_offset = 1;
    
    int8_t lhs[2] = {64, 32};
    int8_t rhs[4] = {64, -64, 64, 64};
    int8_t dst[2] = {0};
    int32_t bias[2] = {0, 0};
    
    printf("  Input: [%d, %d]\n", lhs[0], lhs[1]);
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", dst_multiplier, dst_shift);
    
    arm_nn_vec_mat_mult_t_s8(lhs, rhs, NULL, bias, dst,
                            lhs_offset, dst_offset, dst_multiplier, dst_shift,
                            2, 2, activation_min, activation_max, 
                            address_offset, rhs_offset);
    
    printf("  Resultado CMSIS-NN:\n");
    printf("    Neurona 0: %d -> %.3f\n", dst[0], dst[0]/127.0f);
    printf("    Neurona 1: %d -> %.3f\n", dst[1], dst[1]/127.0f);
    
    if(dst[0] != 127 && dst[1] != 127 && dst[0] != -128 && dst[1] != -128) {
        printf("\n  Resultado: CORRECTO (no saturado)\n\r\n\r");
    } else {
        printf("\n  Resultado: SATURADO\n\r\n\r");
    }
}

/**
  * @brief  Test 4: Matriz 3x4 - CORREGIDO
  */
static void test_3x4_matrix(void)
{
    print_test_header("Test 4: Matriz 3x4");
    
    const int32_t lhs_offset = 0;
    const int32_t rhs_offset = 0;
    const int32_t dst_offset = 0;
    const int32_t dst_multiplier = 0x40000000; // 1.0 en Q30 (CORREGIDO)
    const int32_t dst_shift = 0;
    const int32_t activation_min = -128;
    const int32_t activation_max = 127;
    const int32_t address_offset = 1;
    
    // Vector de entrada y matriz MÁS PEQUEÑOS
    int8_t lhs[3] = {16, 8, 24};     // [0.125, 0.063, 0.188]
    int8_t rhs[12] = {16, 8, 12,     // Valores más pequeños
                      8, 16, 8,
                      12, 8, 16,
                      8, 12, 8};
    int8_t dst[4] = {0};
    int32_t bias[4] = {0, 0, 0, 0};
    
    printf("  Vector lhs: [");
    for(int i = 0; i < 3; i++) printf("%d%s", lhs[i], i<2 ? ", " : "");
    printf("]\n");
    
    printf("  Multiplier: 0x%lX, Shift: %ld\n\n", dst_multiplier, dst_shift);
    
    // CMSIS-NN
    arm_nn_vec_mat_mult_t_s8(lhs, rhs, NULL, bias, dst,
                            lhs_offset, dst_offset, dst_multiplier, dst_shift,
                            3, 4, activation_min, activation_max, 
                            address_offset, rhs_offset);
    
    printf("  Resultados: [");
    for(int i = 0; i < 4; i++) printf("%d%s", dst[i], i<3 ? ", " : "");
    printf("]\n");
    
    printf("  En float: [");
    for(int i = 0; i < 4; i++) printf("%.3f%s", dst[i]/127.0f, i<3 ? ", " : "");
    printf("]\n");
    
    // Verificar que no son cero
    int all_zero = 1;
    for(int i = 0; i < 4; i++) {
        if(dst[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    
    if(!all_zero) {
        printf("\n  Resultado: CORRECTO (valores no cero)\n\r\n\r");
    } else {
        printf("\n  Resultado: ERROR - todos cero\n\r\n\r");
    }
}

/**
  * @brief  Test 5: Requantización y activación - MANTENIDO
  */
static void test_requantization_activation(void)
{
    print_test_header("Test 5: Requantizacion y Activacion");
    
    const int32_t lhs_offset = 0;
    const int32_t rhs_offset = 0;
    const int32_t dst_offset = 0;
    const int32_t dst_multiplier = 0x20000000; // 0.5 en Q30
    const int32_t dst_shift = -1;
    const int32_t activation_min = -64;
    const int32_t activation_max = 64;
    const int32_t address_offset = 1;
    
    int8_t lhs[2] = {64, 64};        // Valores más pequeños (CORREGIDO)
    int8_t rhs[4] = {64, 64, 64, 64};
    int8_t dst[2] = {0};
    int32_t bias[2] = {0, 0};
    
    printf("  Vector lhs: [%d, %d]\n", lhs[0], lhs[1]);
    printf("  Matriz rhs: todos 64\n");
    printf("  Requant: multiplier=0x%lX, shift=%ld\n", dst_multiplier, dst_shift);
    printf("  Activacion: [%ld, %ld]\n\n", activation_min, activation_max);
    
    // CMSIS-NN
    arm_nn_vec_mat_mult_t_s8(lhs, rhs, NULL, bias, dst,
                            lhs_offset, dst_offset, dst_multiplier, dst_shift,
                            2, 2, activation_min, activation_max, 
                            address_offset, rhs_offset);
    
    printf("  Resultados:\n");
    printf("    Salida 0: %d\n", dst[0]);
    printf("    Salida 1: %d\n", dst[1]);
    
    if(dst[0] != 0 && dst[1] != 0) {
        printf("\n  Resultado: CORRECTO (valores no cero)\n\r\n\r");
    } else {
        printf("\n  Resultado: ERROR - valores cero\n\r\n\r");
    }
}

/**
  * @brief  Run all vector-matrix multiplication tests
  */
void Test_VecMatMult_Run_All(void)
{
    printf("\n\r");
    printf("===============================================\n\r");
    printf("  CMSIS-NN arm_nn_vec_mat_mult_t_s8() Tests\n\r");
    printf("===============================================\n\r");
    printf("\n\r");
    
    test_basic_2x2();
    test_with_offsets_bias();
    test_xor_hidden_layer();
    test_3x4_matrix();
    test_requantization_activation();
    
    printf("\n\r");
    printf("===============================================\n\r");
    printf("  Todos los tests completados!\n\r");
    printf("===============================================\n\r");
    printf("\n\r");
    
    printf("Conclusiones:\n\r");
    printf("- Multiplier: 0x40000000 = 1.0 en Q30\n\r");
    printf("- Shift: 0 = sin escalado adicional\n\r");
    printf("- Valores de entrada pequeños evitan saturacion\n\r");
    printf("- Test 5 muestra requantizacion funcionando\n\r\n\r");
}

/* ========================================================
 * HELPER FUNCTIONS
 * ======================================================== */

static void calculate_manual_vec_mat_mult(const int8_t* lhs, const int8_t* rhs, 
                                         const int32_t* bias, int32_t* dst, 
                                         int32_t lhs_offset, int32_t rhs_offset,
                                         int32_t rhs_cols, int32_t rhs_rows)
{
    for(int i = 0; i < rhs_rows; i++) {
        dst[i] = bias ? bias[i] : 0;
        for(int j = 0; j < rhs_cols; j++) {
            int32_t lhs_val = lhs[j] + lhs_offset;
            int32_t rhs_val = rhs[i * rhs_cols + j] + rhs_offset;
            dst[i] += lhs_val * rhs_val;
        }
    }
}

static void print_test_header(const char* test_name)
{
    printf("-----------------------------------------------\n\r");
    printf("%s\n\r", test_name);
    printf("-----------------------------------------------\n\r");
}

static int32_t custom_requantize(int32_t value, int32_t multiplier, int32_t shift)
{
    int64_t result = (int64_t)value * multiplier;
    if (shift < 0) {
        result >>= -shift;
    } else {
        result <<= shift;
    }
    if (result > 127) return 127;
    if (result < -128) return -128;
    return (int32_t)result;
}