#include "xor_float.h"
#include <math.h>

/* ===========================================
 * XOR MODEL PARAMETERS - FLOAT
 * =========================================== */

/* Layer 1: 2→2 weights - FLOAT VERSION */
const float xor_weights_1_float[4] = {
    2.0f,   -2.0f,  /* Neuron 1: detects [1,0] */
    -2.0f,  2.0f    /* Neuron 2: detects [0,1] */
};

/* Layer 1 bias - FLOAT VERSION */
const float xor_bias_1_float[2] = {
    -1.0f, -1.0f
};

/* Layer 2: 2→1 weights - FLOAT VERSION */
const float xor_weights_2_float[2] = {
    1.0f, 1.0f       /* Combine both detectors */
};

/* Layer 2 bias - FLOAT VERSION */
const float xor_bias_2_float[1] = {
    -0.5f            /* Threshold for XOR */
};

/* ===========================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * =========================================== */

/**
 * @brief XOR prediction using float with ReLU and Tanh
 * @param input1 First input value (0.0f or 1.0f)
 * @param input2 Second input value (0.0f or 1.0f) 
 * @return Output after Tanh: -1.0f to 1.0f
 */
float xor_predict_float(float input1, float input2)
{
    /* ================================
     * LAYER 1: 2→2 with ReLU
     * ================================ */
    
    /* Neuron 1 calculation */
    float z1_0 = (input1 * xor_weights_1_float[0]) + 
                 (input2 * xor_weights_1_float[1]) + 
                 xor_bias_1_float[0];
    
    /* Neuron 2 calculation */
    float z1_1 = (input1 * xor_weights_1_float[2]) + 
                 (input2 * xor_weights_1_float[3]) + 
                 xor_bias_1_float[1];
    
    /* ReLU activation */
    float a1_0 = (z1_0 > 0.0f) ? z1_0 : 0.0f;
    float a1_1 = (z1_1 > 0.0f) ? z1_1 : 0.0f;
    
    /* ================================
     * LAYER 2: 2→1 with Tanh
     * ================================ */
    
    /* Output neuron calculation */
    float z2 = (a1_0 * xor_weights_2_float[0]) + 
               (a1_1 * xor_weights_2_float[1]) + 
               xor_bias_2_float[0];
    
    /* Tanh activation */
    float output = tanhf(z2);
    
    return output;
}