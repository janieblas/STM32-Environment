/**
  ******************************************************************************
  * @file    main.c 
  * @brief   UART Printf Example with CAN: retarget printf to UART and CAN
  ******************************************************************************
  */

#include "main.h"

/* Private variables */
UART_HandleTypeDef UartHandle;
CAN_HandleTypeDef hcan1;
uint32_t contador = 0;

/* CAN variables */
CAN_TxHeaderTypeDef   TxHeader;
uint8_t               TxData[8] = {0};
uint32_t              TxMailbox;

/* Private function prototypes */
static void SystemClock_Config(void);
static void UART_Config(void);
static void CAN1_Config(void);
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
  CAN1_Config();
  
  // Configure CAN TX Header 
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.StdId = 0x446;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = 2;
  
  printf("\n\r UART Printf Example with CAN\n\r");
  printf("System started - Sending messages every 1 second...\n\r");

  while (1)
  {
    HAL_Delay(1000);  // Wait 1 second
    contador++;
    
    // Prepare CAN message 
    TxData[0] = contador & 0xFF;  // Lower byte of counter
    TxData[1] = (contador >> 8) & 0xFF;  // Upper byte of counter
    
    // Send CAN message 
    if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
    {
      printf("ERR #%lu: Code=0x%lX ", contador, hcan1.ErrorCode);
      
      // Decode error code
      if (hcan1.ErrorCode & HAL_CAN_ERROR_EWG)
        printf("EWG ");
      if (hcan1.ErrorCode & HAL_CAN_ERROR_EPV)
        printf("EPV ");
      if (hcan1.ErrorCode & HAL_CAN_ERROR_BOF)
      {
        printf("BOF-Restart ");
        HAL_CAN_Stop(&hcan1);
        HAL_Delay(10);
        HAL_CAN_Start(&hcan1);
      }
      if (hcan1.ErrorCode & HAL_CAN_ERROR_TIMEOUT)
        printf("TIMEOUT");
      
      printf("\n\r");
      
      // NO llama Error_Handler() - continúa funcionando
    }
    else
    {
      printf("MSG #%lu OK (MB:%lu)\n\r", contador, TxMailbox);
    }
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
  * @brief  System Clock Configuration
  *         SYSCLK = 80 MHz, HCLK = 80 MHz
  *         APB1 = 20 MHz, APB2 = 40 MHz
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
    printf("ERROR: UART Init failed!\n\r");
    Error_Handler(); 
  }
  else
  {
    printf("UART initialized successfully\n\r");
  }
}

/**
  * @brief  CAN1 Initialization Function
  *         APB1 = 20 MHz
  *         Bitrate = 20MHz / (5 × 16) = 250 kbps
  *         Sample Point = (1 + 13) / 16 = 87.5%
  */
static void CAN1_Config(void)
{
  hcan1.Instance = CAN1;
  //hcan1.Init.Prescaler = 5;               // 250 kbps
  hcan1.Init.Prescaler = 10;               // 125 kbps
  hcan1.Init.Mode = CAN_MODE_NORMAL; //CAN_MODE_NORMAL CAN_MODE_LOOPBACK
  hcan1.Init.SyncJumpWidth = CAN_SJW_2TQ; // Mayor tolerancia
  hcan1.Init.TimeSeg1 = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE ; // Sin reintentos por ahora DISABLE / ENABLE
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    printf("ERROR: CAN Init failed! Error code: 0x%lX\n\r", hcan1.ErrorCode);
    Error_Handler();
  }
  else
  {
    printf("CAN1 initialized successfully\n\r");
  }
  
  /* Start CAN */
  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    printf("ERROR: CAN Start failed! Error code: 0x%lX\n\r", hcan1.ErrorCode);
    Error_Handler();
  }
  else
  {
    printf("CAN1 started successfully (250kbps)\n\r");
  }
}

/**
  * @brief  Error Handler
  */
static void Error_Handler(void)
{
  printf("\n\r!!! FATAL ERROR - System Halted !!!\n\r");
  __disable_irq();
  while(1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
  printf("Assert failed: file %s line %lu\n\r", file, line);
  while (1)
  {
  }
}
#endif