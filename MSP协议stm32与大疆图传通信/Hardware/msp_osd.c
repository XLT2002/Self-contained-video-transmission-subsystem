#include "msp_osd.h"
#include "uart.h"
#include "AD.h"      
#include "usart.h"   
#include <string.h>


// 【终极武器：FIFO 环形队列】
// 保证大疆发来的命令，先来先服务，绝不发生优先级饿死！
#define CMD_FIFO_SIZE 128
volatile uint8_t msp_cmd_fifo[CMD_FIFO_SIZE];
volatile uint8_t msp_fifo_head = 0;
volatile uint8_t msp_fifo_tail = 0;

static void MSP_SendPacket(uint8_t cmd, uint8_t *payload, uint8_t size) {
    uint8_t crc = 0;
    UART2_SendByte('$'); UART2_SendByte('M'); UART2_SendByte('>');  
    UART2_SendByte(size); crc ^= size;
    UART2_SendByte(cmd); crc ^= cmd;
    for (uint8_t i = 0; i < size; i++) { UART2_SendByte(payload[i]); crc ^= payload[i]; }
    UART2_SendByte(crc);
}

// 主循环调用的提取和回复函数
uint8_t MSP_CheckAndReply(void) {
    if (msp_fifo_head == msp_fifo_tail) return 0; // 队列为空，直接返回

    // 提取最早到达的命令
    uint8_t cmd = msp_cmd_fifo[msp_fifo_tail];
    msp_fifo_tail = (msp_fifo_tail + 1) % CMD_FIFO_SIZE;

    // 绝对安全的排队回信
    if (cmd == 1) { 
        uint8_t res[3] = {0, 1, 44}; MSP_SendPacket(1, res, 3);
    } else if (cmd == 2) { 
        uint8_t res[4] = {'B', 'T', 'F', 'L'}; MSP_SendPacket(2, res, 4);
    } else if (cmd == 189) { 
        uint8_t res[2] = {53, 20}; MSP_SendPacket(189, res, 2);
    } else if (cmd == 3) { 
        uint8_t res[3] = {4, 4, 0}; MSP_SendPacket(3, res, 3);
    } else if (cmd == 10) { 
        uint8_t res[5] = {'S','T','M','3','2'}; MSP_SendPacket(10, res, 5);
    } else if (cmd == 101) { 
        uint8_t res[15] = {0}; res[4] = 0x21; res[6] = 0x01; 
        MSP_SendPacket(101, res, 15);
    } else if (cmd == 150) { 
        uint8_t res[16] = {0}; res[4] = 0x21; res[6] = 0x01; 
        MSP_SendPacket(150, res, 16);
    } else if (cmd == 130) { 
        uint8_t res[11] = {2, 0x16, 0x0D, 0, 0, 0, 0, 0, 0, 0, 0}; 
        MSP_SendPacket(130, res, 11);
    } else if (cmd == 110) { 
        uint8_t res[9] = {0}; MSP_SendPacket(110, res, 9);
    } else { 
        uint8_t crc = cmd; 
        UART2_SendByte('$'); UART2_SendByte('M'); UART2_SendByte('!'); 
        UART2_SendByte(0);   UART2_SendByte(cmd); UART2_SendByte(crc);  
    }
    
    return cmd;
}

// 接收状态机 (在中断里光速运行)
void MSP_ProcessRx(uint8_t c) {
    static uint8_t state = 0, offset = 0, dataSize = 0, cmdMSP = 0, checksum = 0, inBuf[64];
    if (state == 0) { if (c == '$') state = 1; return; }
    switch (state) {
        case 1: checksum = 0; state = (c == 'M') ? 2 : 0; break;
        case 2: state = (c == '<') ? 3 : 0; break;
        case 3: inBuf[offset++] = c; checksum ^= c;
            if (offset == 2) { dataSize = inBuf[0]; cmdMSP = inBuf[1]; offset = 0; state = (dataSize > 0) ? 4 : 5; } break;
        case 4: inBuf[offset++] = c; checksum ^= c; if (offset == dataSize) state = 5; break;
        case 5: 
            if (checksum == c) {
                // 【核心修改】：精准按顺序投递入队！
                uint8_t next_head = (msp_fifo_head + 1) % CMD_FIFO_SIZE;
                if (next_head != msp_fifo_tail) { // 如果队列没满
                    msp_cmd_fifo[msp_fifo_head] = cmdMSP;
                    msp_fifo_head = next_head;
                }
            }
            state = 0; offset = 0; break;
        default: state = 0; break;
    }
}

// ============== OSD 画布命令 ==============
void OSD_Heartbeat(void) {
    uint8_t sub_cmd = 0; MSP_SendPacket(182, &sub_cmd, 1);
}
void OSD_Print(uint8_t row, uint8_t col, const char *str) {
    uint8_t payload[64];
    uint8_t len = strlen(str);
    if (len > 30) len = 30;
    payload[0] = 3; payload[1] = row; payload[2] = col; payload[3] = 0; 
    memcpy(&payload[4], str, len);
    MSP_SendPacket(182, payload, len + 4);
}
void OSD_Update(void) {
    uint8_t sub_cmd = 4; MSP_SendPacket(182, &sub_cmd, 1);
}
void OSD_Clear(void) {
    uint8_t sub_cmd = 2; MSP_SendPacket(182, &sub_cmd, 1);
}
