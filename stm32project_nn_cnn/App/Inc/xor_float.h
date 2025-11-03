#ifndef XOR_FLOAT_H
#define XOR_FLOAT_H

#include <stdint.h>

/* ===========================================
 * XOR NEURAL NETWORK - FLOAT IMPLEMENTATION
 * Architecture: 2 → 2 → 1
 * Activations: ReLU (hidden), Tanh (output)
 * =========================================== */

/* Layer 1 Parameters (2→2 with ReLU) - FLOAT VERSION */
extern const float xor_weights_1_float[4];
extern const float xor_bias_1_float[2];

/* Layer 2 Parameters (2→1 with Tanh) - FLOAT VERSION */
extern const float xor_weights_2_float[2];
extern const float xor_bias_2_float[1];

/* ===========================================
 * FUNCTION PROTOTYPES
 * =========================================== */

/**
 * @brief XOR prediction using float with ReLU and Tanh
 * @param input1 First input value (0.0f or 1.0f)
 * @param input2 Second input value (0.0f or 1.0f) 
 * @return Output after Tanh: -1.0f to 1.0f
 */
float xor_predict_float(float input1, float input2);

#endif /* XOR_FLOAT_H */