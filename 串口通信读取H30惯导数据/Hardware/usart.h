#ifndef __USART_H
#define __USART_H

void H30_UART1_Init(u32 bound);
extern float Pitch, Roll, Yaw;//角度
extern float AccX, AccY, AccZ;//加速度 
extern float GyroX, GyroY, GyroZ;//角速度

#endif
