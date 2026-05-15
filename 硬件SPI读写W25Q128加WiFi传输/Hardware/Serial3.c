#include "stm32f10x.h"                  
#include "Serial3.h"

char ESP_RX_Buf[256];
uint16_t ESP_RX_Count = 0;

// 初始化串口3 (PB10 TX, PB11 RX)
void Serial3_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // PB10 TX 初始化为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // PB11 RX 初始化为上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // USART3 配置为 115200 波特率
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART3, &USART_InitStructure);
    
    // 开启 USART3 接收中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART3, ENABLE);
}

void Serial3_SendByte(uint8_t Byte) {
    USART_SendData(USART3, Byte);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void Serial3_SendString(char *String) {
    uint8_t i = 0;
    while (String[i] != '\0') {
        Serial3_SendByte(String[i++]);
    }
}

// 专门用来发送 16进制/二进制 传感器数据的函数（遇到 0x00 也不会停止）
void Serial3_SendArray(uint8_t *Array, uint16_t Length) {
    for (uint16_t i = 0; i < Length; i++) {
        Serial3_SendByte(Array[i]);
    }
}

void Serial3_ClearBuf(void) {
    for(int i = 0; i < 256; i++) ESP_RX_Buf[i] = 0;
    ESP_RX_Count = 0;
}

// USART3 中断服务函数（硬件自动调用）
void USART3_IRQHandler(void) {
    if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
        uint8_t rx_data = USART_ReceiveData(USART3);
        // 防止数组越界
        if (ESP_RX_Count < 255) {
            ESP_RX_Buf[ESP_RX_Count++] = rx_data;
            ESP_RX_Buf[ESP_RX_Count] = '\0'; // 始终保持字符串结尾
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}
