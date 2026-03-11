#include "main.h"
#include "system_config.h"
#include "x_detector.h"

/*==============================================================================
 * FILE INFORMATION
 *============================================================================*/
/**
 * @file    x_detector.h
 * @author  Antonio Nieblas
 * @email   nibj961116@gmail.com
 * @date    November 05, 2025
 * @brief   X pattern detector header file
 */
 
/*==============================================================================
 * LOCAL CONSTANTS
 *============================================================================*/

/**
 * @brief Test pattern for X detection (5x5 matrix)
 */
static const int8_t TEST_PATTERN_X[25] = 
{
    1, 0, 0, 0, 1,
    0, 1, 0, 1, 0,  
    0, 0, 1, 0, 0,
    0, 1, 0, 1, 0,
    1, 0, 0, 0, 1
};

/*==============================================================================
 * FUNCTION DEFINITIONS
 *============================================================================*/

/**
 * @brief Main application entry point
 * @return int Not used in embedded application
 */
int main(void)
{
    detection_result_t detection_result;
    
    /* System initialization */
    HAL_Init();
    SystemClock_Config();
    UART_Config();
    
    /* Application header */
    printf("\n\r==========================================\n\r");
    printf("X PATTERN DETECTOR - CMSIS-NN Convolution\n\r");
    printf("==========================================\n\r");
    
    /* Initialize X detector */
    x_detector_init();
    
    printf("\n\r--- X DETECTOR TEST ---\n\r");
    
    /* Test X pattern detection */
    printf("\n\r>>> Testing X pattern:\n\r");
    detection_result = x_detect(TEST_PATTERN_X);
    
    /* Display final result */
    printf("\n\r=== FINAL RESULT ===\n\r");
    if (detection_result.prediction == 1)
    {
        printf("SUCCESS: X pattern correctly detected\n\r");
        printf("Confidence score: %d\n\r", detection_result.score);
    }
    else
    {
        printf("FAIL: X pattern not detected\n\r");
        printf("Confidence score: %d\n\r", detection_result.score);
    }
    
    printf("\n\r==========================================\n\r");
    printf("Test completed. System running.\n\r");
    printf("==========================================\n\r");

    /* Main application loop */
    while (1) 
    {
        HAL_Delay(1000U);
    }
    
    return 0;
}