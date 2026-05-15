#include "stm32f10x.h"
#include "Serial3.h"
#include "Delay.h"
#include "Serial.h"    // 引入 CH340 串口打印库替代 OLED
#include <string.h>
#include <stdio.h>

// 初始化 ESP-01S 为 AP 热点 + TCP 服务器
void ESP8266_Init_Server(void) {
    Serial3_Init();
    
    Serial_Printf("\r\n[WiFi] System Initializing...\r\n");
    
    // 1. 设置为路由器模式
    Serial3_ClearBuf();
    Serial3_SendString("AT+CWMODE=2\r\n");
    Delay_ms(1000); 
    Serial_Printf("[WiFi] CWMODE Ret: %s", ESP_RX_Buf); // 打印 ESP 返回值
    
    // 2. 建立 Wi-Fi 热点 (名称 xlt_haha, 密码 12345678)
    Serial3_ClearBuf();
    Serial3_SendString("AT+CWSAP=\"xlt_haha\",\"12345678\",1,3\r\n");
    Delay_ms(2000); 
    Serial_Printf("[WiFi] CWSAP Ret: %s", ESP_RX_Buf);
    
    // 3. 允许多连接 (开启服务器的前提)
    Serial3_ClearBuf();
    Serial3_SendString("AT+CIPMUX=1\r\n");
    Delay_ms(500);
    Serial_Printf("[WiFi] CIPMUX Ret: %s", ESP_RX_Buf);
    
    // 4. 开启 TCP 服务器，端口 8080
    Serial3_ClearBuf();
    Serial3_SendString("AT+CIPSERVER=1,8080\r\n");
    Delay_ms(500);
    Serial_Printf("[WiFi] CIPSERVER Ret: %s", ESP_RX_Buf);
    
    Serial_Printf("\r\n=========================================\r\n");
    Serial_Printf(" >> WiFi AP Ready! SSID: xlt_haha\r\n");
    Serial_Printf(" >> TCP Server Port: 8080\r\n");
    Serial_Printf("=========================================\r\n");
}

// 检查是否收到了电脑发来的 "DOWNLOAD" 指令
uint8_t ESP8266_CheckDownloadCommand(void) {
    if (strstr(ESP_RX_Buf, "DOWNLOAD") != NULL) {
        Serial3_ClearBuf(); 
        return 1; 
    }
    return 0; 
}

// 通过 TCP 向电脑发送数据包
void ESP8266_Send_TCP_Data(uint8_t *data, uint16_t len) {
    char cmd[32];
    sprintf(cmd, "AT+CIPSEND=0,%d\r\n", len);
    Serial3_SendString(cmd);
    
    Delay_ms(50); 
    
    Serial3_SendArray(data, len);
    Delay_ms(50); 
}
