#ifndef X_DETECTOR_H
#define X_DETECTOR_H

/*==============================================================================
 * INCLUDES
 *============================================================================*/

#include <stdint.h>
#include "arm_nnfunctions.h"

/*==============================================================================
 * DEFINES
 *============================================================================*/

#define INPUT_SIZE    (5U)
#define FILTER_SIZE   (3U) 
#define OUTPUT_SIZE   (3U)

/*==============================================================================
 * TYPEDEFS
 *============================================================================*/

/**
 * @brief Detection result structure
 */
typedef struct 
{
    int8_t score;      /**< Detection confidence score */
    int8_t prediction; /**< Detection result: 0 = Not X, 1 = X detected */
} detection_result_t;

/*==============================================================================
 * FUNCTION PROTOTYPES  
 *============================================================================*/

/**
 * @brief Initialize the X pattern detector
 */
void x_detector_init(void);

/**
 * @brief Detect X pattern in input matrix
 * @param input_matrix Pointer to input matrix data (5x5)
 * @return Detection result structure
 */
detection_result_t x_detect(const int8_t* input_matrix);

#endif /* X_DETECTOR_H */