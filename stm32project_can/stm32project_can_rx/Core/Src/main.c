#include "main.h"
#include "system_clock.h"
#include "uart_debug.h"
#include "can_bus.h"

UART_HandleTypeDef UartHandle;
CAN_HandleTypeDef  hcan1;

static uint32_t msgCount  = 0;

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
  CAN_Init();

  printf("\n\r=== CAN Receptor 500kbps  222 ===\n\r");
  printf("Esperando mensajes...\n\r");

  while (1)
  {
    uint32_t id, dlc;
    uint8_t  data[8] = {0};

    if (CAN_ReceiveMessage(&id, data, &dlc))
    {
      msgCount++;
      uint16_t valor = data[0] | (data[1] << 8);
      printf("RX #%lu | ID=0x%lX DLC=%lu | %02X %02X | Val=%u\n\r",
             msgCount, id, dlc, data[0], data[1], valor);
    }

    static uint32_t lastReport = 0;
    if (HAL_GetTick() - lastReport > 5000)
    {
      lastReport = HAL_GetTick();
      CAN_PrintStatus();
    }
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  printf("Assert: %s line %lu\n\r", file, line);
  while (1) {}
}
#endif