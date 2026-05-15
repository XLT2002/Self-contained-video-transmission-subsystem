#include "uart.h"
#include "msp_osd.h" // 引入 MSP 模块以在中断中调用接收解析

/**
 * @brief 初始化 USART2 (PA2=TX, PA3=RX), 波特率 115200
 */
void USART2_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    // TX: PA2 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX: PA3 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 115200, 8-N-1
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    // 开启接收中断
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART2, ENABLE);
}

/**
 * @brief 阻塞发送单个字节
 */
void UART2_SendByte(uint8_t byte) {
    USART_SendData(USART2, byte);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

/**
 * @brief 串口中断，将接收到的字节直接送入 MSP 状态机
 */
void USART2_IRQHandler(void) {
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        uint8_t c = USART_ReceiveData(USART2);
        // 将数据丢给严格的 Betaflight MSP 状态机处理
        MSP_ProcessRx(c);
    }
	// 2. 【核心补丁：清理 ORE 溢出错误】
    // 如果发生了溢出错误，必须通过连续读取 SR 和 DR 寄存器来清除标志位，
    // 否则单片机会陷入死循环卡死，或者彻底不再接收数据！
    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) {
        USART_ReceiveData(USART2); // 空读一次数据寄存器，这是 STM32 硬件手册规定的清除 ORE 的唯一正确方法
    }
    
    // 3. 清理可能出现的其他线路错误 (噪音干扰导致的帧错误 FE、噪声错误 NE 等)
    if (USART_GetFlagStatus(USART2, USART_FLAG_FE) != RESET || 
        USART_GetFlagStatus(USART2, USART_FLAG_NE) != RESET || 
        USART_GetFlagStatus(USART2, USART_FLAG_PE) != RESET) {
        USART_ReceiveData(USART2); // 同样空读一次来清除这些错误
    }
}
