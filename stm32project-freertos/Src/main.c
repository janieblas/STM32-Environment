/**
  ******************************************************************************
  * @file    main.c
  * @author  Your Name
  * @brief   FreeRTOS application with LED blink and UART output tasks
  * @version 1.0
  * @date    2025-09-28
  ******************************************************************************
  * @attention
  *
  * Target: STM32F4xx with FreeRTOS
  * LED: PA5 (500ms toggle period)
  * UART: 9600 baud, 8N1
  *
  ******************************************************************************
  */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef UartHandle;
GPIO_InitTypeDef GPIO_InitStruct = {0};

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void UART_Config(void);
static void GPIO_Config(void);
static void Error_Handler(void);
void vBlinkTask(void *pvParameters);
void vUARTTask(void *pvParameters);
int __io_putchar(int ch);

/* Main application ----------------------------------------------------------*/

/**
  * @brief  Application entry point
  * @retval None
  */
int main(void)
{
  /* Initialize HAL library and configure system */
  HAL_Init();
  SystemClock_Config();
  UART_Config();
  GPIO_Config();

  printf("\n\r=== FreeRTOS Example: LED + UART Tasks ===\n\r");

  /* Create FreeRTOS tasks */
  BaseType_t status;
  
  status = xTaskCreate(vBlinkTask, "Blink", 256, NULL, 1, NULL);
  if (status != pdPASS) {
    printf("ERROR: BlinkTask not created!\n\r");
  }

  status = xTaskCreate(vUARTTask, "UART", 256, NULL, 1, NULL);
  if (status != pdPASS) {
    printf("ERROR: UARTTask not created!\n\r");
  }

  /* Start FreeRTOS scheduler - this function never returns */
  vTaskStartScheduler();

  /* Execution should never reach this point */
  for(;;);
}

/* Task implementations ------------------------------------------------------*/

/**
  * @brief  LED blink task
  * @param  pvParameters: Task parameters (unused)
  * @note   Toggles PA5 every 500ms with periodic status output
  * @retval None
  */
void vBlinkTask(void *pvParameters)
{
  uint32_t counter = 0;
  
  for(;;)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

    /* Print status every full cycle (1 second) */
    if (counter % 2 == 0)
    {
      printf("Blink #%lu - LED %s - HAL_GetTick(): %lu ms\n\r", 
             counter/2 + 1,
             (counter % 4 == 0) ? "ON " : "OFF", 
             HAL_GetTick());
    }

    counter++;
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/**
  * @brief  UART heartbeat task
  * @param  pvParameters: Task parameters (unused)
  * @note   Outputs system tick every 1 second
  * @retval None
  */
void vUARTTask(void *pvParameters)
{
  for(;;)
  {
    printf("UART Task alive! tick = %lu ms\n\r", HAL_GetTick());
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/* HAL Configuration ---------------------------------------------------------*/

/**
  * @brief  Retargets printf to UART
  * @param  ch: Character to send
  * @retval Character sent
  */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF); 
  return ch;
}

/**
  * @brief  Configure GPIO for LED output
  * @note   Configures PA5 as push-pull output for onboard LED
  * @retval None
  */
static void GPIO_Config(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

/**
  * @brief  Configure system clocks
  * @note   SYSCLK: 180 MHz | AHB: 180 MHz | APB1: 45 MHz | APB2: 90 MHz
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Configure HSI oscillator and PLL */
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

  /* Enable overdrive mode for 180 MHz operation */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Configure system clock sources and dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                 RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
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
  * @note   9600 baud, 8 data bits, no parity, 1 stop bit
  * @retval None
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
  * @brief  System error handler
  * @note   Enters infinite loop with rapid LED blinking
  * @retval None
  */
static void Error_Handler(void)
{
  printf("ERROR: System Error Handler Called!\n\r");
  
  while(1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(100);
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports assertion failure
  * @param  file: Source file name
  * @param  line: Line number where assertion failed
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  printf("ASSERT FAILED: file %s on line %d\n\r", file, line);
  while (1) { }
}
#endif