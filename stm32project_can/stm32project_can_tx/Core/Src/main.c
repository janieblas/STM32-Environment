#include "main.h"
#include "system_clock.h"
#include "uart_debug.h"
#include "can_bus.h"

UART_HandleTypeDef UartHandle;
CAN_HandleTypeDef  hcan1;

static uint32_t contador = 0;

void Error_Handler(void)
{
  printf("\n\r!!! FATAL ERROR !!!\n\r");
  __disable_irq();
  while (1) {}
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  UART_Init();

  printf("SYSCLK = %lu Hz\n\r", HAL_RCC_GetSysClockFreq());
  printf("APB1   = %lu Hz\n\r", HAL_RCC_GetPCLK1Freq());

  CAN_Init();

  printf("\n\r=== CAN Transmisor 222 ===\n\r");
  printf("Esperando 5 segundos...\n\r");
  HAL_Delay(5000);
  printf("Iniciando transmision\n\r");

  while (1)
  {
    HAL_Delay(1000);
    contador++;
    CAN_SendMessage(CAN_TX_ID, contador);
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  printf("Assert: %s line %lu\n\r", file, line);
  while (1) {}
}
#endif