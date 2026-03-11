#include "can_bus.h"

extern CAN_HandleTypeDef hcan1;

void CAN_Init(void)
{
  hcan1.Instance                  = CAN1;
  hcan1.Init.Prescaler            = 5;
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

uint8_t CAN_ReceiveMessage(uint32_t *id, uint8_t *data, uint32_t *dlc)
{
  if (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) == 0)
    return 0;

  CAN_RxHeaderTypeDef RxHeader;
  if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxHeader, data) != HAL_OK)
    return 0;

  *id  = RxHeader.StdId;
  *dlc = RxHeader.DLC;
  return 1;
}

void CAN_PrintStatus(void)
{
  uint8_t  tec = (CAN1->ESR >> 16) & 0xFF;
  uint8_t  rec = (CAN1->ESR >> 24) & 0xFF;
  uint32_t esr = CAN1->ESR;
  printf("--- CAN: TEC=%u REC=%u ESR=0x%lX ---\n\r", tec, rec, esr);
}