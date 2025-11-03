#ifndef XOR_MODEL_H
#define XOR_MODEL_H

#include "arm_nnfunctions.h"
#include "arm_nn_types.h"

/* ===========================================
 * XOR NEURAL NETWORK MODEL PARAMETERS
 * Architecture: 2 → 2 → 1
 * Quantization: Q7 weights, Q31 bias
 * =========================================== */

/* Layer 1 Parameters (2→2 with ReLU) */
extern const int8_t xor_weights_1[4];
extern const int32_t xor_bias_1[2];

/* Layer 1 Quantization Parameters */
#define XOR_MULTIPLIER_1  (0x0CCCCCCC)  /* 0.100000 */
#define XOR_SHIFT_1       (0)

/* Layer 2 Parameters (2→1 with threshold) */
extern const int8_t xor_weights_2[2];
extern const int32_t xor_bias_2[1];

/* Layer 2 Quantization Parameters */
#define XOR_MULTIPLIER_2  (0x050F6F12)  /* 0.039534 */
#define XOR_SHIFT_2       (0)

/* ===========================================
 * FUNCTION PROTOTYPES
 * =========================================== */

/**
 * @brief Initialize XOR model parameters and CMSIS-NN contexts
 */
void xor_model_init(void);

/**
 * @brief XOR prediction - complete model inference
 * @param input1 First input value (0-127)
 * @param input2 Second input value (0-127)
 * @return Binary output: 127 (true) or 0 (false)
 */
int8_t xor_predict(int8_t input1, int8_t input2);

#endif /* XOR_MODEL_H */