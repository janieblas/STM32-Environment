#ifndef AND_MODEL_H
#define AND_MODEL_H

#include "arm_nnfunctions.h"

// ================================
// PARÁMETROS DEL MODELO AND
// ================================

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

// Parámetros del modelo AND (1 neurona)
extern const int8_t and_weights[];
extern const int32_t and_bias[];

// PARÁMETROS CUANTIZACIÓN OPTIMIZADOS
#define AND_MULTIPLIER 0x0204D86D  // ← MULTIPLICADOR QUE FUNCIONA
#define AND_SHIFT 0

// Prototipos de funciones
void and_model_init(void);
void debug_and_internal_activations(int8_t input1, int8_t input2);
void debug_and_quantization_params(void);
void debug_and_simple_calculation(void);
void calculate_and_quant_params(void);
int8_t manual_and_predict(int8_t input1, int8_t input2);
int8_t and_predict(int8_t input1, int8_t input2);
void test_and_model(void);

#endif