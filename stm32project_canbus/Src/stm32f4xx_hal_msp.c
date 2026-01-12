/**
  ******************************************************************************
  * @file    UART/UART_Printf/Src/stm32f4xx_hal_msp.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.    
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();
  
  /* Enable USARTx clock */
  USARTx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;
  
  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);

}

/**
  * @brief CAN MSP Initialization
  *        This function configures the hardware resources used for CAN:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration (PB8=RX, PB9=TX para CAN1)
  * @param hcan: CAN handle pointer
  * @retval None
  */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  if(hcan->Instance == CAN1)
  {
    /*##-1- Enable peripherals and GPIO Clocks ###############################*/
    /* Enable GPIO clock */
    CANx_GPIO_CLK_ENABLE();
    
    /* Enable CAN1 clock */
    CANx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ########################################*/
    /* CAN1 TX GPIO pin configuration (PB9) */
    GPIO_InitStruct.Pin       = CANx_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD; //GPIO_MODE_AF_PP GPIO_MODE_AF_OD
    GPIO_InitStruct.Pull      = GPIO_NOPULL; // GPIO_NOPULL
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = CANx_TX_AF;
    HAL_GPIO_Init(CANx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* CAN1 RX GPIO pin configuration (PB8) */
    GPIO_InitStruct.Pin       = CANx_RX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = CANx_RX_AF;
    HAL_GPIO_Init(CANx_RX_GPIO_PORT, &GPIO_InitStruct);

    printf("  CAN1 MSP Init: GPIO PB8/PB9 configured\n\r");
    printf("  CAN1 MSP Init: CAN1 clock enabled\n\r");
  }
}

/**
  * @brief CAN MSP De-Initialization
  *        This function frees the hardware resources used for CAN:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to their default state
  * @param hcan: CAN handle pointer
  * @retval None
  */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
  if(hcan->Instance == CAN1)
  {
    /*##-1- Reset peripherals ################################################*/
    CANx_FORCE_RESET();
    CANx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ##############################*/
    /* Deconfigure CAN TX GPIO pin */
    HAL_GPIO_DeInit(CANx_TX_GPIO_PORT, CANx_TX_PIN);
    /* Deconfigure CAN RX GPIO pin */
    HAL_GPIO_DeInit(CANx_RX_GPIO_PORT, CANx_RX_PIN);
  }
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */