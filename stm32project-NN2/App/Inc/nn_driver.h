/**
  ******************************************************************************
  * @file    nn_driver.h
  * @brief   Neural Network Driver for XOR - Q7 Implementation
  ******************************************************************************
  */
#ifndef __NN_DRIVER_H
#define __NN_DRIVER_H

#include <stdint.h>

/* Network architecture configuration */
#define INPUT_SIZE 2
#define HIDDEN_SIZE 8
#define OUTPUT_SIZE 1

/* Public function prototypes */
void NN_Init(void);
float NN_Predict(const float input[2]);
void NN_Test_XOR(void);
void NN_Print_Configuration(void);

#endif /* __NN_DRIVER_H */