#ifndef __CAN_BUS_H
#define __CAN_BUS_H

#include "stm32f4xx_hal.h"
#include <stdio.h>

#define CAN_BITRATE  500  // kbps

void     CAN_Init(void);
uint8_t  CAN_ReceiveMessage(uint32_t *id, uint8_t *data, uint32_t *dlc);
void     CAN_PrintStatus(void);

#endif