#ifndef TEST_TANH_ACTIVATION_H
#define TEST_TANH_ACTIVATION_H

#include "arm_nnfunctions.h"

void Test_Tanh_Activation_Run_All(void);
void debug_tanh_behavior(void);
void test_tanh_positive_negative(void);
void test_tanh_saturation(void);
void test_tanh_vs_relu_comparison(void);
void test_tanh_with_different_multipliers(void);

#endif