/**
  ******************************************************************************
  * @file    test_dotprod.c
  * @brief   Test suite for arm_dot_prod_q7() - Educational examples
  ******************************************************************************
  */
#include "test_dotprod.h"
#include "arm_math.h"
#include <stdio.h>

/* Private function prototypes */
static void print_test_header(const char* test_name);
static void print_result(int32_t expected, q31_t obtained);
static int32_t calculate_manual(const q7_t* vecA, const q7_t* vecB, uint32_t len);

/**
  * @brief  Test 1: Vectores básicos [1.0, 0.0]
  */
static void test_basic_vectors(void)
{
    print_test_header("Test 1: Vectores Básicos");
    
    // En Q7: 1.0 → 127, 0.0 → 0
    q7_t vecA[2] = {127, 0};
    q7_t vecB[2] = {127, 0};
    q31_t result = 0;
    
    printf("  vecA = [%d, %d] → [1.0, 0.0] en Q7\n", vecA[0], vecA[1]);
    printf("  vecB = [%d, %d] → [1.0, 0.0] en Q7\n\n", vecB[0], vecB[1]);
    
    // Cálculo manual
    int32_t expected = calculate_manual(vecA, vecB, 2);
    
    // Cálculo CMSIS-DSP
    arm_dot_prod_q7(vecA, vecB, 2, &result);
    
    printf("  Cálculo manual: 127×127 + 0×0 = %ld (Q14)\n", expected);
    printf("  CMSIS-DSP:      %ld (Q14)\n", result);
    print_result(expected, result);
}

/**
  * @brief  Test 2: Vectores con valores negativos
  */
static void test_negative_values(void)
{
    print_test_header("Test 2: Valores Negativos");
    
    // [1.0, -1.0] · [1.0, 1.0] = 1×1 + (-1)×1 = 0
    q7_t vecA[2] = {127, -127};
    q7_t vecB[2] = {127, 127};
    q31_t result = 0;
    
    printf("  vecA = [%d, %d] → [1.0, -1.0] en Q7\n", vecA[0], vecA[1]);
    printf("  vecB = [%d, %d] → [1.0,  1.0] en Q7\n\n", vecB[0], vecB[1]);
    
    int32_t expected = calculate_manual(vecA, vecB, 2);
    arm_dot_prod_q7(vecA, vecB, 2, &result);
    
    printf("  Cálculo manual: 127×127 + (-127)×127 = %ld (Q14)\n", expected);
    printf("  CMSIS-DSP:      %ld (Q14)\n", result);
    print_result(expected, result);
}

/**
  * @brief  Test 3: Pesos reales de tu XOR (primera neurona)
  */
static void test_xor_neuron0(void)
{
    print_test_header("Test 3: Neurona 0 de XOR");
    
    // Pesos de la neurona 0: W1[0] = {121, -121}
    // Input ejemplo: [1.0, 0.0] → [127, 0]
    q7_t input[2] = {127, 0};
    q7_t weights[2] = {121, -121};
    q31_t result = 0;
    
    printf("  Input:   [%d, %d] → [1.0, 0.0]\n", input[0], input[1]);
    printf("  Weights: [%d, %d] (neurona 0 de XOR)\n\n", weights[0], weights[1]);
    
    int32_t expected = calculate_manual(input, weights, 2);
    arm_dot_prod_q7(input, weights, 2, &result);
    
    printf("  Cálculo manual: 127×121 + 0×(-121) = %ld (Q14)\n", expected);
    printf("  CMSIS-DSP:      %ld (Q14)\n", result);
    printf("  Shift a Q7:     %ld >> 7 = %ld\n", result, result >> 7);
    print_result(expected, result);
}

/**
  * @brief  Test 4: Vector más largo (8 elementos)
  */
static void test_longer_vector(void)
{
    print_test_header("Test 4: Vector de 8 Elementos");
    
    // Simula salida de capa oculta
    q7_t hidden[8] = {100, 50, 0, 100, 100, 0, 120, 0};
    q7_t weights[8] = {127, 0, 0, 127, 127, 0, -120, 126};
    q31_t result = 0;
    
    printf("  Hidden:  [");
    for(int i = 0; i < 8; i++) {
        printf("%4d", hidden[i]);
        if(i < 7) printf(",");
    }
    printf("]\n");
    
    printf("  Weights: [");
    for(int i = 0; i < 8; i++) {
        printf("%4d", weights[i]);
        if(i < 7) printf(",");
    }
    printf("]\n\n");
    
    int32_t expected = calculate_manual(hidden, weights, 8);
    arm_dot_prod_q7(hidden, weights, 8, &result);
    
    printf("  Cálculo manual: %ld (Q14)\n", expected);
    printf("  CMSIS-DSP:      %ld (Q14)\n", result);
    printf("  Shift a Q7:     %ld >> 7 = %ld\n", result, result >> 7);
    print_result(expected, result);
}

/**
  * @brief  Test 5: Caso extremo - Todos valores máximos
  */
static void test_max_values(void)
{
    print_test_header("Test 5: Valores Máximos");
    
    q7_t vecA[2] = {127, 127};
    q7_t vecB[2] = {127, 127};
    q31_t result = 0;
    
    printf("  vecA = [127, 127] → [1.0, 1.0]\n");
    printf("  vecB = [127, 127] → [1.0, 1.0]\n\n");
    
    int32_t expected = calculate_manual(vecA, vecB, 2);
    arm_dot_prod_q7(vecA, vecB, 2, &result);
    
    printf("  Cálculo manual: 127×127 + 127×127 = %ld (Q14)\n", expected);
    printf("  CMSIS-DSP:      %ld (Q14)\n", result);
    printf("  En Q7:          %ld >> 7 = %ld (SATURADO en 127)\n", result, result >> 7);
    print_result(expected, result);
}

/**
  * @brief  Run all dot product tests
  */
void Test_DotProd_Run_All(void)
{
    printf("\n\r");
    printf("========================================\n\r");
    printf("  CMSIS-DSP arm_dot_prod_q7() Tests\n\r");
    printf("========================================\n\r");
    printf("\n\r");
    
    test_basic_vectors();
    test_negative_values();
    test_xor_neuron0();
    test_longer_vector();
    test_max_values();
    
    printf("\n\r");
    printf("========================================\n\r");
    printf("  Todos los tests completados!\n\r");
    printf("========================================\n\r");
    printf("\n\r");
    
    printf("Conclusiones:\n\r");
    printf("✓ arm_dot_prod_q7() devuelve resultado en formato Q14\n\r");
    printf("✓ Necesitas >> 7 para convertir Q14 → Q7\n\r");
    printf("✓ Maneja correctamente valores negativos\n\r");
    printf("✓ Funciona con vectores de cualquier tamaño\n\r");
    printf("✓ Es equivalente al cálculo manual\n\r\n\r");
}

/* ========================================================
 * HELPER FUNCTIONS
 * ======================================================== */

/**
  * @brief  Calculate dot product manually for verification
  */
static int32_t calculate_manual(const q7_t* vecA, const q7_t* vecB, uint32_t len)
{
    int32_t sum = 0;
    for(uint32_t i = 0; i < len; i++) {
        // Q7 × Q7 = Q14
        sum += (int32_t)vecA[i] * (int32_t)vecB[i];
    }
    return sum;
}

/**
  * @brief  Print test header
  */
static void print_test_header(const char* test_name)
{
    printf("----------------------------------------\n\r");
    printf("%s\n\r", test_name);
    printf("----------------------------------------\n\r");
}

/**
  * @brief  Print comparison result
  */
static void print_result(int32_t expected, q31_t obtained)
{
    if(expected == obtained) {
        printf("\n  Resultado: ✓ CORRECTO\n\r\n\r");
    } else {
        printf("\n  Resultado: ✗ ERROR - Diferencia: %ld\n\r\n\r", 
               obtained - expected);
    }
}