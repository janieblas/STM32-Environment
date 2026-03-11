/**
  ******************************************************************************
  * @file    main.h 
  * @brief   UART Printf Example: retarget printf to UART
  ******************************************************************************
  */
#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"
#include "stdio.h"

// USART2 configuration
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

// USART2 pins
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2

#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

// CAN1 configuration
#define CANx                             CAN1
#define CANx_CLK_ENABLE()                __HAL_RCC_CAN1_CLK_ENABLE()
#define CANx_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()

#define CANx_FORCE_RESET()               __HAL_RCC_CAN1_FORCE_RESET()
#define CANx_RELEASE_RESET()             __HAL_RCC_CAN1_RELEASE_RESET()

// CAN1 pins (PB8 = RX, PB9 = TX)
#define CANx_TX_PIN                      GPIO_PIN_9
#define CANx_TX_GPIO_PORT                GPIOB
#define CANx_TX_AF                       GPIO_AF9_CAN1

#define CANx_RX_PIN                      GPIO_PIN_8
#define CANx_RX_GPIO_PORT                GPIOB
#define CANx_RX_AF                       GPIO_AF9_CAN1

#endif /* __MAIN_H */