#include "x_detector.h"
#include <stdio.h>

/*==============================================================================
 * LOCAL CONSTANTS
 *============================================================================*/

/**
 * @brief X pattern detection filter (3x3)
 */
static const int8_t FILTER_X[9] = 
{
    20, -15, 20,   /* Strong diagonals */
    -15, 25, -15,  /* Strong center, negative surroundings */
    20, -15, 20    /* Strong diagonals */
};

/**
 * @brief Zero bias for convolution
 */
static const int32_t ZERO_BIAS[1] = {0};

/**
 * @brief Quantization parameters (no scaling)
 */
static const int32_t MULTIPLIERS[1] = {1073741824}; /* 1.0 in Q31 */
static const int32_t SHIFTS[1] = {0}; /* No shift */

/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/

static cmsis_nn_context context;
static cmsis_nn_conv_params conv_params;
static cmsis_nn_per_channel_quant_params quant_params;
static cmsis_nn_dims input_dims;
static cmsis_nn_dims filter_dims;
static cmsis_nn_dims output_dims;
static cmsis_nn_dims bias_dims;
static cmsis_nn_dims upscale_dims;

static int32_t kernel_buffer[32];
static int8_t output_data[OUTPUT_SIZE * OUTPUT_SIZE * 1];

/*==============================================================================
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/

static void initialize_dimensions(void);
static void initialize_convolution_parameters(void);

/*==============================================================================
 * FUNCTION DEFINITIONS
 *============================================================================*/

/**
 * @brief Initialize the X pattern detector
 */
void x_detector_init(void)
{
    /* Initialize context */
    context.buf = kernel_buffer;
    context.size = (int32_t)sizeof(kernel_buffer);
    
    /* Initialize dimensions and parameters */
    initialize_dimensions();
    initialize_convolution_parameters();
    
    /* Configure quantization */
    quant_params.multiplier = (int32_t*)MULTIPLIERS;
    quant_params.shift = (int32_t*)SHIFTS;
    
    printf("X detector initialized\n");
}

/**
 * @brief Detect X pattern in input matrix
 * @param input_matrix Pointer to input matrix data (5x5)
 * @return Detection result structure
 */
detection_result_t x_detect(const int8_t* input_matrix)
{
    detection_result_t result = {0, 0};
    arm_cmsis_nn_status status;
    int32_t sum;
    uint8_t i;
    
    /* Execute convolution */
    status = arm_convolve_s8(&context,
                            &conv_params,
                            &quant_params,
                            &input_dims,
                            input_matrix,
                            &filter_dims,
                            FILTER_X,
                            &bias_dims,
                            ZERO_BIAS,
                            &upscale_dims,
                            &output_dims,
                            output_data);
    
    if (status == ARM_CMSIS_NN_SUCCESS)
    {
        /* Calculate average score */
        sum = 0;
        for (i = 0; i < (OUTPUT_SIZE * OUTPUT_SIZE); i++)
        {
            sum += output_data[i];
        }
        
        result.score = (int8_t)(sum / (OUTPUT_SIZE * OUTPUT_SIZE));
        
        /* Decision: if score > threshold, it's X */
        if (result.score > 7)
        {
            result.prediction = 1; /* X detected */
            printf("X detected - Score: %d\n", result.score);
        }
        else
        {
            result.prediction = 0; /* Not X */
            printf("Not X - Score: %d\n", result.score);
        }
    }
    else
    {
        printf("Convolution error: %d\n", status);
        result.prediction = 0; /* Default: not X */
    }
    
    return result;
}

/*==============================================================================
 * LOCAL FUNCTION DEFINITIONS
 *============================================================================*/

/**
 * @brief Initialize CMSIS-NN dimensions
 */
static void initialize_dimensions(void)
{
    /* Input dimensions */
    input_dims.n = 1;
    input_dims.h = INPUT_SIZE;
    input_dims.w = INPUT_SIZE;
    input_dims.c = 1;
    
    /* Filter dimensions */
    filter_dims.n = 1;  /* 1 filter (X only) */
    filter_dims.h = FILTER_SIZE;
    filter_dims.w = FILTER_SIZE;
    filter_dims.c = 1;
    
    /* Output dimensions */
    output_dims.n = 1;
    output_dims.h = OUTPUT_SIZE;
    output_dims.w = OUTPUT_SIZE;
    output_dims.c = 1;  /* 1 output channel */
    
    /* Bias dimensions */
    bias_dims.n = 1;
    bias_dims.h = 1;
    bias_dims.w = 1;
    bias_dims.c = 1;
    
    /* Upscale dimensions (zero for normal convolution) */
    upscale_dims.n = 0;
    upscale_dims.h = 0;
    upscale_dims.w = 0;
    upscale_dims.c = 0;
}

/**
 * @brief Initialize convolution parameters
 */
static void initialize_convolution_parameters(void)
{
    /* Convolution parameters */
    conv_params.padding.h = 0;
    conv_params.padding.w = 0;
    conv_params.stride.h = 1;
    conv_params.stride.w = 1;
    conv_params.dilation.h = 1;
    conv_params.dilation.w = 1;
    conv_params.input_offset = 0;
    conv_params.output_offset = 0;
    conv_params.activation.min = -128;
    conv_params.activation.max = 127;
}