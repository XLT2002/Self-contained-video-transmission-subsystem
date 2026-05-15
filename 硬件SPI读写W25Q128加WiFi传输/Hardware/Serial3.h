#ifndef __SERIAL3_H
#define __SERIAL3_H              

extern char ESP_RX_Buf[256];  // 暴露给外部的接收缓冲区
extern uint16_t ESP_RX_Count; // 接收计数器

void Serial3_Init(void);
void Serial3_SendByte(uint8_t Byte);
void Serial3_SendString(char *String);
void Serial3_SendArray(uint8_t *Array, uint16_t Length);
void Serial3_ClearBuf(void);

#endif
