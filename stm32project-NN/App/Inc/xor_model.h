#ifndef XOR_MODEL_H
#define XOR_MODEL_H

#include "arm_nnfunctions.h"

// ================================
// PARÁMETROS DEL MODELO XOR
// ================================

// Declaraciones extern (definiciones estarán en .c file)
extern const int8_t hidden_weights[];
extern const int32_t hidden_bias[];
extern const int8_t output_weights[];  // Changed to extern
extern const int32_t output_bias[];

// Parámetros de cuantización
#define HIDDEN_MULTIPLIER 0x7F000000
#define HIDDEN_SHIFT -7
#define OUTPUT_MULTIPLIER 0x60000000                     
#define OUTPUT_SHIFT -5

// Prototipos de funciones
void xor_model_init(void);
void debug_internal_activations(int8_t input1, int8_t input2);
void debug_output_layer(int8_t* hidden_activations);
void debug_quantization_params(void);
void debug_simple_calculation(void);
int8_t manual_xor_predict(int8_t input1, int8_t input2);
int8_t xor_predict(int8_t input1, int8_t input2);
void test_xor_model(void);

#endif