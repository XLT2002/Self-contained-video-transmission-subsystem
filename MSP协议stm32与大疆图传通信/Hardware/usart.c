#include "stm32f10x.h"
#include "usart.h"
#include <stdint.h>
#include <stdio.h>

// 1. 定义全局变量
float Pitch = 0.0, Roll = 0.0, Yaw = 0.0;
float AccX = 0.0, AccY = 0.0, AccZ = 0.0;     
float GyroX = 0.0, GyroY = 0.0, GyroZ = 0.0;  

// 2. 状态机解析缓冲区
u16 rx_state = 0;
u8 payload_len = 0;        // 有效数据域的总长度
u8 payload_buffer[255];    // 暂存有效数据的数组
u8 payload_index = 0;

// 3. 串口 1 初始化函数 (保持 460800 不变)
void H30_UART1_Init(u32 bound) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = bound; 
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 
    USART_Cmd(USART1, ENABLE);                     
}

// 4. 串口 1 中断服务函数 (Yesense 协议解析)
void USART1_IRQHandler(void) {
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        u8 res = USART_ReceiveData(USART1); 

        // 状态 0: 捕捉帧头1 0x59
        if (rx_state == 0 && res == 0x59) {
            rx_state = 1;
        } 
        // 状态 1: 捕捉帧头2 0x53
        else if (rx_state == 1 && res == 0x53) {
            rx_state = 2;
        } 
        // 状态 2, 3: 丢弃帧序号 TID (2字节)
        else if (rx_state == 2 || rx_state == 3) {
            rx_state++;
        } 
        // 状态 4: 获取数据域长度 LEN
        else if (rx_state == 4) {
            payload_len = res;
            payload_index = 0;
            rx_state = 5;
        } 
        // 状态 5: 提取核心数据到 Payload 缓冲区
        else if (rx_state == 5) {
            payload_buffer[payload_index++] = res;
            if (payload_index >= payload_len) {
                rx_state = 6; // 数据域接收完毕，进入校验位
            }
        } 
        // 状态 6, 7: 丢弃校验位，并开始解析 Payload
        else if (rx_state == 6) {
            rx_state = 7;
        } 
        else if (rx_state == 7) {
            int i = 0;
            // 遍历整个数据域，寻找我们需要的模块
            while (i < payload_len) {
                u8 data_id = payload_buffer[i++]; // 获取模块 ID
                u8 data_len = payload_buffer[i++]; // 获取该模块长度

                // 模块：加速度 (ID: 0x10)
                if (data_id == 0x10 && data_len == 12) {
                    int32_t ax = (int32_t)(payload_buffer[i] | (payload_buffer[i+1]<<8) | (payload_buffer[i+2]<<16) | (payload_buffer[i+3]<<24));
                    int32_t ay = (int32_t)(payload_buffer[i+4] | (payload_buffer[i+5]<<8) | (payload_buffer[i+6]<<16) | (payload_buffer[i+7]<<24));
                    int32_t az = (int32_t)(payload_buffer[i+8] | (payload_buffer[i+9]<<8) | (payload_buffer[i+10]<<16) | (payload_buffer[i+11]<<24));
                    AccX = ax * 0.000001f;
                    AccY = ay * 0.000001f;
                    AccZ = az * 0.000001f;
                    i += 12;
                } 
                // 模块：角速度 (ID: 0x20)
                else if (data_id == 0x20 && data_len == 12) {
                    int32_t gx = (int32_t)(payload_buffer[i] | (payload_buffer[i+1]<<8) | (payload_buffer[i+2]<<16) | (payload_buffer[i+3]<<24));
                    int32_t gy = (int32_t)(payload_buffer[i+4] | (payload_buffer[i+5]<<8) | (payload_buffer[i+6]<<16) | (payload_buffer[i+7]<<24));
                    int32_t gz = (int32_t)(payload_buffer[i+8] | (payload_buffer[i+9]<<8) | (payload_buffer[i+10]<<16) | (payload_buffer[i+11]<<24));
                    GyroX = gx * 0.000001f;
                    GyroY = gy * 0.000001f;
                    GyroZ = gz * 0.000001f;
                    i += 12;
                } 
                // 模块：欧拉角 (ID: 0x40)
                else if (data_id == 0x40 && data_len == 12) {
                    int32_t pit_raw = (int32_t)(payload_buffer[i] | (payload_buffer[i+1]<<8) | (payload_buffer[i+2]<<16) | (payload_buffer[i+3]<<24));
                    int32_t rol_raw = (int32_t)(payload_buffer[i+4] | (payload_buffer[i+5]<<8) | (payload_buffer[i+6]<<16) | (payload_buffer[i+7]<<24));
                    int32_t yaw_raw = (int32_t)(payload_buffer[i+8] | (payload_buffer[i+9]<<8) | (payload_buffer[i+10]<<16) | (payload_buffer[i+11]<<24));
                    Pitch = pit_raw * 0.000001f;
                    Roll = rol_raw * 0.000001f;
                    Yaw = yaw_raw * 0.000001f;
                    i += 12;
                } 
                // 其他未关心的模块 (如磁力计、四元数等)，直接跳过其长度
                else {
                    i += data_len; 
                }
            }
            rx_state = 0; // 整个长帧解析完成，重置状态机
        } 
        else {
            rx_state = 0; // 发生任何时序错误，立即重置
        }
    }
}



/**
  * 函    数：重定向 c 库函数 printf 到 USART1
  * 说    明：可以在 main 里面用 printf 打印了
  */
int fputc(int ch, FILE *f)
{
    // 将字符通过串口 1 (PA9) 发送出去
    USART_SendData(USART1, (uint8_t) ch);
    // 等待发送完毕
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);		
    return ch;
}
