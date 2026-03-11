#ifndef __CAN_BUS_H
#define __CAN_BUS_H

#include "stm32f4xx_hal.h"
#include <stdio.h>

#define CAN_TX_ID    0x446
#define CAN_BITRATE  500  // kbps

void CAN_Init(void);
void CAN_SendMessage(uint32_t id, uint32_t contador);

#endif