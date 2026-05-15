#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"     // 必须包含
#include "W25Q64.h"
#include "ESP8266.h"

uint8_t ArrayWrite[16] = "Hello FancyXLT!"; // 测试数据
uint8_t ArrayRead[16] = {0};                 
uint8_t is_downloading = 0; 

int main(void) {
    Serial_Init();      // 初始化 CH340 串口观测台
    W25Q64_Init();      // 初始化 SPI 存储器
    
    // 1. 初始化 Wi-Fi 模块
    ESP8266_Init_Server();
    
    // 2. 将测试数据写入存储器
    Serial_Printf("[Flash] Erasing Sector 0...\r\n");
    W25Q64_SectorErase(0x000000); 
    Serial_Printf("[Flash] Writing Data...\r\n");
    W25Q64_PageProgram(0x000000, ArrayWrite, 16); 
    Delay_ms(500);
    
    Serial_Printf("[SYS] Write Done. Standing by for commands...\r\n");

    while (1) {
        // ========== 状态 A：等待电脑指令 ==========
        if (is_downloading == 0) {
            if (ESP8266_CheckDownloadCommand() == 1) {
                is_downloading = 1; 
                Serial_Printf("\r\n[!!!] Command 'DOWNLOAD' Received!\r\n");
                Serial_Printf("[SYS] Reading from W25Q128 and transmitting...\r\n");
            }
            Delay_ms(100); 
        } 
        // ========== 状态 B：执行下载任务 ==========
        else if (is_downloading == 1) {
            // 从黑匣子读取数据
            W25Q64_ReadData(0x000000, ArrayRead, 16);
            
            // 通过 Wi-Fi 模块发送
            ESP8266_Send_TCP_Data(ArrayRead, 16);
            
            Serial_Printf("[SUCCESS] Transmission Complete! Data sent: %s\r\n", ArrayRead);
            is_downloading = 0; 
        }
    }
}
