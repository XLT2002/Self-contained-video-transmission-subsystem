#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

void USART2_Init(void);
void UART2_SendByte(uint8_t byte);

#endif
