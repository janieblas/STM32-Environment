/**
  ******************************************************************************
  * @file    nn_driver.c
  * @brief   Neural Network Driver for XOR - Q7 Implementation
  ******************************************************************************
  */
#include "nn_driver.h"
#include "main.h"
#include <math.h>
#include <stdio.h>

/* Factor de escala de cuantización */
#define SCALE_FACTOR 85.2606f

/* ============================================================
 * PESOS CUANTIZADOS Q7 (int8_t)
 * ============================================================ */

// W1: Input -> Hidden [2x8]
static const int8_t W1_q7[INPUT_SIZE][HIDDEN_SIZE] = {
    { 121,  -28,  -50,  121,  121,  -43,  127, -121},
    {-121,  -49,  -54, -121, -121,  -21, -114,  121}
};

// b1: Bias Hidden [8]
static const int8_t b1_q7[HIDDEN_SIZE] = {0, 0, 0, 0, 0, 0, 114, 0};

// W2: Hidden -> Output [8x1]
static const int8_t W2_q7[HIDDEN_SIZE][OUTPUT_SIZE] = {
    { 127},
    {   0},
    {   0},
    { 127},
    { 127},
    {   0},
    {-120},
    { 126}
};

// b2: Bias Output [1]
static const int8_t b2_q7[OUTPUT_SIZE] = {-10};

/* Test inputs for XOR */
static const float test_inputs[4][2] = {
    {0.0f, 0.0f},  // Expected: 0
    {0.0f, 1.0f},  // Expected: 1
    {1.0f, 0.0f},  // Expected: 1
    {1.0f, 1.0f}   // Expected: 0
};

/* Private function prototypes */
static float sigmoid_approx(float x);

/**
  * @brief  Neural Network Initialization
  */
void NN_Init(void)
{
    // Puedes agregar inicializaciones futuras aquí
    printf("Neural Network Driver Initialized\n\r");
}

/**
  * @brief  Predict XOR output - Implementación manual Q7
  */
float NN_Predict(const float input[2])
{
    int8_t input_q7[INPUT_SIZE];
    int8_t hidden_q7[HIDDEN_SIZE];
    int8_t output_q7[OUTPUT_SIZE];
    int32_t acc;
    
    // ========================================================
    // PASO 1: Convertir entrada flotante -> Q7
    // ========================================================
    for (int i = 0; i < INPUT_SIZE; i++) {
        input_q7[i] = (int8_t)(input[i] * 127.0f);
    }
    
    // ========================================================
    // PASO 2: CAPA OCULTA (Hidden Layer)
    // ========================================================
    for (int h = 0; h < HIDDEN_SIZE; h++)
    {
        acc = 0;
        
        // Multiplicación: Input × Weights (Q7 × Q7 = Q14)
        for (int i = 0; i < INPUT_SIZE; i++)
        {
            acc += (int32_t)input_q7[i] * (int32_t)W1_q7[i][h];
        }
        
        // Agregar bias (Q7, escalar a Q14)
        acc += (int32_t)b1_q7[h] << 7;
        
        // Shift Q14 -> Q7 (dividir por 128)
        acc = acc >> 7;
        
        // Saturar a rango Q7 [-128, 127]
        if (acc > 127) acc = 127;
        if (acc < -128) acc = -128;
        
        // ReLU: max(0, x)
        hidden_q7[h] = (acc < 0) ? 0 : (int8_t)acc;
    }
    
    // ========================================================
    // PASO 3: CAPA DE SALIDA (Output Layer)
    // ========================================================
    acc = 0;
    
    // Multiplicación: Hidden × Weights (Q7 × Q7 = Q14)
    for (int h = 0; h < HIDDEN_SIZE; h++)
    {
        acc += (int32_t)hidden_q7[h] * (int32_t)W2_q7[h][0];
    }
    
    // Agregar bias (Q7, escalar a Q14)
    acc += (int32_t)b2_q7[0] << 7;
    
    // Shift Q14 -> Q7
    acc = acc >> 7;
    
    // Saturar a rango Q7
    if (acc > 127) acc = 127;
    if (acc < -128) acc = -128;
    
    output_q7[0] = (int8_t)acc;
    
    // ========================================================
    // PASO 4: Sigmoid
    // ========================================================
    // Convertir Q7 -> flotante normalizado [-1, 1]
    float output_float = (float)output_q7[0] / 127.0f;
    
    // Aplicar sigmoid
    float output_sigmoid = sigmoid_approx(output_float);
    
    return output_sigmoid;
}

/**
  * @brief  Aproximación de Sigmoid
  */
static float sigmoid_approx(float x)
{
    // Sigmoid estándar: 1 / (1 + e^(-x))
    // Escalamos x para mejor rango
    return 1.0f / (1.0f + expf(-x * 4.0f));
}

/**
  * @brief  Print Network Configuration
  */
void NN_Print_Configuration(void)
{
    printf("Network Configuration:\n\r");
    printf("- Input neurons: %d\n\r", INPUT_SIZE);
    printf("- Hidden neurons: %d (ReLU)\n\r", HIDDEN_SIZE);
    printf("- Output neurons: %d (Sigmoid)\n\r", OUTPUT_SIZE);
    printf("- Precision: Q7 (int8_t)\n\r");
    printf("- Scale factor: %.2f\n\r\n\r", SCALE_FACTOR);
    
    printf("Pesos W1 (Input -> Hidden):\n\r");
    for(int i = 0; i < INPUT_SIZE; i++) {
        printf("  [");
        for(int h = 0; h < HIDDEN_SIZE; h++) {
            printf("%4d", W1_q7[i][h]);
            if(h < HIDDEN_SIZE-1) printf(",");
        }
        printf("]\n\r");
    }
    
    printf("\nBias b1: [");
    for(int h = 0; h < HIDDEN_SIZE; h++) {
        printf("%4d", b1_q7[h]);
        if(h < HIDDEN_SIZE-1) printf(",");
    }
    printf("]\n\r");
    
    printf("\nPesos W2 (Hidden -> Output):\n\r");
    for(int h = 0; h < HIDDEN_SIZE; h++) {
        printf("  [%4d]\n\r", W2_q7[h][0]);
    }
    
    printf("\nBias b2: [%4d]\n\r\n\r", b2_q7[0]);
}

/**
  * @brief  Test XOR Neural Network with all inputs
  */
void NN_Test_XOR(void)
{
    printf("Testing XOR Neural Network:\n\r");
    printf("----------------------------\n\r");
    
    int correct_predictions = 0;
    
    for(int i = 0; i < 4; i++)
    {
        float prediction = NN_Predict(test_inputs[i]);
        int expected = (int)((test_inputs[i][0] != test_inputs[i][1]) ? 1 : 0);
        int predicted_class = (prediction > 0.5f) ? 1 : 0;
        char result = (predicted_class == expected) ? 'V' : 'X';
        
        if(predicted_class == expected) {
            correct_predictions++;
        }
        
        printf("Input: [%.0f, %.0f] -> Output: %.4f -> Class: %d [%c]\n\r", 
                test_inputs[i][0], 
                test_inputs[i][1],
                prediction,
                predicted_class,
                result);
    }
    
    printf("\n\rResults Summary:\n\r");
    printf("----------------\n\r");
    printf("Correct predictions: %d/4\n\r", correct_predictions);
    printf("Accuracy: %.1f%%\n\r", (correct_predictions / 4.0f) * 100.0f);
    
    printf("\n\rTesting complete!\n\r");
}