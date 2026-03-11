#ifndef __UART_DEBUG_H
#define __UART_DEBUG_H

#include "stm32f4xx_hal.h"
#include "main.h"
#include <stdio.h>

void UART_Init(void);
int  __io_putchar(int ch);

#endif