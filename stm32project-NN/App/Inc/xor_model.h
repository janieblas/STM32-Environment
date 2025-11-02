#ifndef XOR_MODEL_H
#define XOR_MODEL_H

#include "arm_nnfunctions.h"
#include "arm_nn_types.h"

// ================================
// PARÁMETROS DEL MODELO XOR - CAPA 1
// ================================

// Parámetros de la capa 1 (2→2)
extern const int8_t xor_weights_1[];
extern const int32_t xor_bias_1[];

// Multiplicador de Python
#define XOR_MULTIPLIER_1 0x0CCCCCCC  // 0.100000 de Python
#define XOR_SHIFT_1 0

// ================================
// PARÁMETROS DEL MODELO XOR - CAPA 2
// ================================

// Parámetros de la capa 2 (2→1)  
extern const int8_t xor_weights_2[];
extern const int32_t xor_bias_2[];

// Multiplicador capa 2 de Python
#define XOR_MULTIPLIER_2 0x050F6F12    // 0.039534 de Python
#define XOR_SHIFT_2 0

// ================================
// PROTOTIPOS DE FUNCIONES
// ================================

// Inicialización
void xor_model_init(void);
void calculate_xor_quant_params(void);

// Pruebas de capa 1
void test_xor_layer1_only(void);
void get_xor_layer1_output(int8_t input1, int8_t input2, int8_t* hidden_output);
void manual_xor_layer1_predict(int8_t input1, int8_t input2, int8_t* output0, int8_t* output1);

// Pruebas completas
void test_xor_complete_model(void);
int8_t xor_predict(int8_t input1, int8_t input2);

// Debug
void debug_xor_quantization_params(void);
void debug_xor_layer1_activations(int8_t input1, int8_t input2);
void debug_xor_complete_flow(int8_t input1, int8_t input2);

#endif