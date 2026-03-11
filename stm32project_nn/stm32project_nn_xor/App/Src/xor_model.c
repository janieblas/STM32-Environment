#include "xor_model.h"
#include <stdint.h>

/* ===========================================
 * XOR MODEL PARAMETERS
 * =========================================== */

/* Layer 1: 2→2 weights (Q7) */
const int8_t xor_weights_1[4] = {
    20, -20,   /* Neuron 1 weights */
    -20, 20    /* Neuron 2 weights */
};

/* Layer 1 bias (Q31) */
const int32_t xor_bias_1[2] = {
    -10, -10
};

/* Layer 2: 2→1 weights (Q7) */
const int8_t xor_weights_2[2] = {
    10, 10
};

/* Layer 2 bias (Q31) */
const int32_t xor_bias_2[1] = {
    -5
};

/* ===========================================
 * CMSIS-NN CONTEXTS AND PARAMETERS
 * =========================================== */

/* Layer 1 context and parameters */
static cmsis_nn_context xor_ctx_1;
static cmsis_nn_fc_params xor_fc_params_1;
static cmsis_nn_per_tensor_quant_params xor_quant_params_1;
static cmsis_nn_dims xor_input_dims;
static cmsis_nn_dims xor_filter_dims_1;
static cmsis_nn_dims xor_bias_dims_1;
static cmsis_nn_dims xor_hidden_dims;

/* Layer 2 context and parameters */
static cmsis_nn_context xor_ctx_2;
static cmsis_nn_fc_params xor_fc_params_2;
static cmsis_nn_per_tensor_quant_params xor_quant_params_2;
static cmsis_nn_dims xor_filter_dims_2;
static cmsis_nn_dims xor_bias_dims_2;
static cmsis_nn_dims xor_output_dims;

/* Output buffer */
static int8_t xor_final_output[1];

/* ===========================================
 * PRIVATE FUNCTION PROTOTYPES
 * =========================================== */

static void get_xor_layer1_output(int8_t input1, int8_t input2, int8_t* hidden_output);

/* ===========================================
 * PUBLIC FUNCTION IMPLEMENTATIONS
 * =========================================== */

/**
 * @brief Initialize XOR model parameters and CMSIS-NN contexts
 */
void xor_model_init(void)
{
    /* Layer 1 buffers */
    static int32_t kernel_sum_buffer_1[2] = {0};
    
    /* Layer 2 buffers */
    static int32_t kernel_sum_buffer_2[1] = {0};
    
    /* ================================
     * LAYER 1 INITIALIZATION (2→2)
     * ================================ */
    
    /* Context initialization */
    xor_ctx_1.size = sizeof(kernel_sum_buffer_1);
    xor_ctx_1.buf = kernel_sum_buffer_1;
    
    /* Quantization parameters */
    xor_quant_params_1.multiplier = XOR_MULTIPLIER_1;
    xor_quant_params_1.shift = XOR_SHIFT_1;
    
    /* Fully connected parameters */
    xor_fc_params_1.input_offset = 0;
    xor_fc_params_1.filter_offset = 0;
    xor_fc_params_1.output_offset = 0;
    xor_fc_params_1.activation.min = 0;     /* ReLU activation */
    xor_fc_params_1.activation.max = 127;
    
    /* Layer 1 dimensions */
    xor_input_dims.n = 1;
    xor_input_dims.h = 1;
    xor_input_dims.w = 1;
    xor_input_dims.c = 2;
    
    xor_filter_dims_1.n = 2;
    xor_filter_dims_1.h = 1;
    xor_filter_dims_1.w = 1;
    xor_filter_dims_1.c = 2;
    
    xor_bias_dims_1.n = 1;
    xor_bias_dims_1.h = 1;
    xor_bias_dims_1.w = 1;
    xor_bias_dims_1.c = 2;
    
    xor_hidden_dims.n = 1;
    xor_hidden_dims.h = 1;
    xor_hidden_dims.w = 1;
    xor_hidden_dims.c = 2;
    
    /* ================================
     * LAYER 2 INITIALIZATION (2→1)
     * ================================ */
    
    /* Context initialization */
    xor_ctx_2.size = sizeof(kernel_sum_buffer_2);
    xor_ctx_2.buf = kernel_sum_buffer_2;
    
    /* Quantization parameters */
    xor_quant_params_2.multiplier = XOR_MULTIPLIER_2;
    xor_quant_params_2.shift = XOR_SHIFT_2;
    
    /* Fully connected parameters */
    xor_fc_params_2.input_offset = 0;
    xor_fc_params_2.filter_offset = 0;
    xor_fc_params_2.output_offset = 0;
    
    /* CRITICAL NOTE: Layer 2 activation settings
     * 
     * We use clipping instead of tanh activation for binary classification because:
     * 1. XOR is a binary function (0 or 1) - we don't need smooth tanh outputs
     * 2. Clipping is more efficient than tanh computation on microcontrollers
     * 3. The threshold is applied externally in the application layer
     * 4. This follows CMSIS-NN recommendations for binary classification
     * 
     * For regression or multi-class, use arm_nn_activation_s16() instead
     */
    xor_fc_params_2.activation.min = -128;  /* No lower clipping */
    xor_fc_params_2.activation.max = 127;   /* No upper clipping */
    
    /* CRITICAL NOTE: Layer 2 dimensions explanation
     * 
     * Layer 2: 2→1 (2 inputs → 1 output)
     * - filter_dims.n = 2: TWO inputs per output neuron (not number of neurons!)
     * - filter_dims.c = 1: ONE output neuron
     * 
     * Why n=2 and not n=1?
     * In CMSIS-NN, for fully connected layers:
     * - n dimension represents INPUT features per output neuron
     * - c dimension represents OUTPUT neurons count
     * 
     * This matches the weight matrix shape: [output_neurons, input_features]
     * For 2→1 layer: weights shape is [1, 2] but stored as [2, 1] in memory
     */
    xor_filter_dims_2.n = 2;  /* 2 input features per output neuron */
    xor_filter_dims_2.h = 1;
    xor_filter_dims_2.w = 1;
    xor_filter_dims_2.c = 1;  /* 1 output neuron */
    
    xor_bias_dims_2.n = 1;
    xor_bias_dims_2.h = 1;
    xor_bias_dims_2.w = 1;
    xor_bias_dims_2.c = 1;
    
    xor_output_dims.n = 1;
    xor_output_dims.h = 1;
    xor_output_dims.w = 1;
    xor_output_dims.c = 1;
}

/**
 * @brief XOR prediction - complete model inference
 * @param input1 First input value (0-127)
 * @param input2 Second input value (0-127)
 * @return Raw output value (application should apply threshold: >0 = true)
 * 
 * Note: Binary threshold should be applied externally:
 * - output > 0 → XOR true (inputs are different)
 * - output <= 0 → XOR false (inputs are same)
 */
int8_t xor_predict(int8_t input1, int8_t input2)
{
    int8_t hidden_output[2];
    
    /* Layer 1: 2→2 with ReLU */
    get_xor_layer1_output(input1, input2, hidden_output);
    
    /* Layer 2: 2→1 with clipping (no activation) */
    arm_fully_connected_s8(&xor_ctx_2, 
                          &xor_fc_params_2, 
                          &xor_quant_params_2,
                          &xor_hidden_dims, 
                          hidden_output,
                          &xor_filter_dims_2, 
                          xor_weights_2,
                          &xor_bias_dims_2, 
                          xor_bias_2,
                          &xor_output_dims, 
                          xor_final_output);
    
    return xor_final_output[0];
}

/* ===========================================
 * PRIVATE FUNCTION IMPLEMENTATIONS
 * =========================================== */

/**
 * @brief Get layer 1 hidden output for given inputs
 * @param input1 First input value
 * @param input2 Second input value
 * @param hidden_output Output buffer for hidden layer
 */
static void get_xor_layer1_output(int8_t input1, int8_t input2, int8_t* hidden_output)
{
    int8_t input_data[2] = {input1, input2};
    
    /* Layer 1: 2→2 with ReLU */
    arm_fully_connected_s8(&xor_ctx_1,
                          &xor_fc_params_1,
                          &xor_quant_params_1,
                          &xor_input_dims,
                          input_data,
                          &xor_filter_dims_1,
                          xor_weights_1,
                          &xor_bias_dims_1,
                          xor_bias_1,
                          &xor_hidden_dims,
                          hidden_output);
}