/**
  ******************************************************************************
  * @file    main.c 
  * @brief   UART Printf Example: retarget printf to UART
  ******************************************************************************
  */

#include "main.h"

/* Private variables */
UART_HandleTypeDef UartHandle;
GPIO_InitTypeDef GPIO_InitStruct = {0};

/* Private function prototypes */
static void SystemClock_Config(void);
static void UART_Config(void);
static void GPIO_Config(void);
static void Error_Handler(void);
int __io_putchar(int ch);

/**
  * @brief  Main program
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  UART_Config();
  GPIO_Config();
  
  printf("\n\r=== UART Printf + LED Blink Example ===\n\r");
  printf("Using TIM6 as HAL timebase (SysTick free for FreeRTOS)\n\r");
  printf("LED Blinking every 500ms on PA5\n\r\n\r");

  uint32_t counter = 0;

  while (1)
  {
    // Toggle LED
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    
    // Print counter every second (every 2 toggles)
    if (counter % 2 == 0) 
    {
      printf("Blink #%lu - LED %s - HAL_GetTick(): %lu ms\n\r", 
             counter/2 + 1,
             (counter % 4 == 0) ? "ON " : "OFF", 
             HAL_GetTick());
    }
    
    // Delay 500ms using TIM6-based HAL_Delay
    HAL_Delay(500);
    
    counter++;
  }
}

/**
  * @brief  Retargets printf to USART
  */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF); 
  return ch;
}

/**
  * @brief  Configure GPIO for LED
  *         PA5 = User LED (on Nucleo boards)
  */
static void GPIO_Config(void)
{
  // Enable GPIO Clock
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  // Configure PA5 as output push-pull
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  // Start with LED OFF
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
  * @brief  System Clock Configuration
  *         SYSCLK = 180 MHz, HCLK = 180 MHz
  *         APB1 = 45 MHz, APB2 = 90 MHz
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSI and PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 6;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Enable OverDrive for 180 MHz */  
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Configure system clocks */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  Configure UART peripheral
  */
static void UART_Config(void)
{
  UartHandle.Instance          = USARTx;
  UartHandle.Init.BaudRate     = 9600;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler(); 
  }
}

/**
  * @brief  Error Handler
  */
static void Error_Handler(void)
{
  printf("ERROR: System Error Handler Called!\n\r");
  while(1)
  {
    // Blink LED rapidly to indicate error
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(100);
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  printf("ASSERT FAILED: file %s on line %d\n\r", file, line);
  while (1)
  {
  }
}
#endif