#include "can_bus.h"

extern CAN_HandleTypeDef hcan1;

static CAN_TxHeaderTypeDef TxHeader;
static uint8_t             TxData[8] = {0};
static uint32_t            TxMailbox;

void CAN_Init(void)
{
  hcan1.Instance                  = CAN1;
  hcan1.Init.Prescaler            = 5;  // 500kbps | 20 = 125kbps
  hcan1.Init.Mode                 = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth        = CAN_SJW_2TQ;
  hcan1.Init.TimeSeg1             = CAN_BS1_13TQ;
  hcan1.Init.TimeSeg2             = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode    = DISABLE;
  hcan1.Init.AutoBusOff           = ENABLE;
  hcan1.Init.AutoWakeUp           = DISABLE;
  hcan1.Init.AutoRetransmission   = ENABLE;
  hcan1.Init.ReceiveFifoLocked    = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;

  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    printf("ERROR: CAN Init! 0x%lX\n\r", hcan1.ErrorCode);
    Error_Handler();
  }
  printf("CAN1 initialized\n\r");

  CAN_FilterTypeDef canFilter;
  canFilter.FilterBank           = 0;
  canFilter.FilterMode           = CAN_FILTERMODE_IDMASK;
  canFilter.FilterScale          = CAN_FILTERSCALE_32BIT;
  canFilter.FilterIdHigh         = 0x0000;
  canFilter.FilterIdLow          = 0x0000;
  canFilter.FilterMaskIdHigh     = 0x0000;
  canFilter.FilterMaskIdLow      = 0x0000;
  canFilter.FilterFIFOAssignment = CAN_RX_FIFO0;
  canFilter.FilterActivation     = ENABLE;

  if (HAL_CAN_ConfigFilter(&hcan1, &canFilter) != HAL_OK)
  {
    printf("ERROR: CAN Filter!\n\r");
    Error_Handler();
  }
  printf("CAN1 filter OK\n\r");

  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    printf("ERROR: CAN Start! 0x%lX\n\r", hcan1.ErrorCode);
    Error_Handler();
  }
  printf("CAN1 started - %dkbps\n\r", CAN_BITRATE);
}

void CAN_SendMessage(uint32_t id, uint32_t contador)
{
  TxHeader.IDE   = CAN_ID_STD;
  TxHeader.StdId = id;
  TxHeader.RTR   = CAN_RTR_DATA;
  TxHeader.DLC   = 2;

  TxData[0] = contador & 0xFF;
  TxData[1] = (contador >> 8) & 0xFF;

  uint8_t tec = (CAN1->ESR >> 16) & 0xFF;
  uint8_t rec = (CAN1->ESR >> 24) & 0xFF;

  if (HAL_CAN_AddTxMessage(&hcan1, &TxHeader, TxData, &TxMailbox) != HAL_OK)
  {
    printf("ERR #%lu Code=0x%lX TEC=%u REC=%u\n\r",
           contador, hcan1.ErrorCode, tec, rec);
  }
  else
  {
    printf("TX #%lu OK (MB:%lu) Data=%02X %02X TEC=%u REC=%u\n\r",
           contador, TxMailbox, TxData[0], TxData[1], tec, rec);
  }
}