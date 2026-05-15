#include "stm32f10x.h"                  
#include "Delay.h"
#include "Serial.h"   // 引入串口库替代 OLED
#include "W25Q64.h"   // 底层驱动文件名不变，兼容 W25Q128

uint8_t MID;          // 存放制造商 ID
uint16_t DID;         // 存放设备 ID

// 测试数据：准备写进去的 4 个字节
uint8_t ArrayWrite[] = {0xAA, 0xBB, 0x33, 0x44};	
// 测试数据：准备读出来的 4 个字节存放点
uint8_t ArrayRead[4] = {0};								

int main(void)
{
    // 1. 模块初始化
    Serial_Init();      // 初始化串口1 (PA9 TX, PA10 RX)
    W25Q64_Init();      // 初始化底层硬件 SPI 接口
    
    // 打印测试表头
    Serial_Printf("\r\n=========================================\r\n");
    Serial_Printf("      W25Q128 SPI Flash Test V1.0        \r\n");
    Serial_Printf("=========================================\r\n");
    
    // 2. 读取芯片 ID 身份证
    W25Q64_ReadID(&MID, &DID);			
    Serial_Printf(">> 1. Reading Chip ID...\r\n");
    Serial_Printf("      MID: 0x%02X (Winbond: 0xEF)\r\n", MID);
    // W25Q64的DID是4017，W25Q128的容量大一倍，DID是4018
    Serial_Printf("      DID: 0x%04X (W25Q128: 0x4018)\r\n", DID); 
    Serial_Printf("-----------------------------------------\r\n");
    
    // 3. 写之前先擦除扇区
    Serial_Printf(">> 2. Erasing Sector 0 (Address: 0x000000)...\r\n");
    W25Q64_SectorErase(0x000000);
    Serial_Printf("      Erase Done!\r\n");
    
    // 4. 写入测试数据
    Serial_Printf(">> 3. Writing Data to Address 0x000000...\r\n");
    Serial_Printf("      Write: 0x%02X, 0x%02X, 0x%02X, 0x%02X\r\n", 
                  ArrayWrite[0], ArrayWrite[1], ArrayWrite[2], ArrayWrite[3]);
    W25Q64_PageProgram(0x000000, ArrayWrite, 4);
    Serial_Printf("      Write Done!\r\n");
    
    // 5. 读出刚刚写入的数据验证
    Serial_Printf(">> 4. Reading Data back...\r\n");
    W25Q64_ReadData(0x000000, ArrayRead, 4);
    Serial_Printf("      Read : 0x%02X, 0x%02X, 0x%02X, 0x%02X\r\n", 
                  ArrayRead[0], ArrayRead[1], ArrayRead[2], ArrayRead[3]);
    
    // 6. 自动比对验证结果
    if(ArrayRead[0] == ArrayWrite[0] && ArrayRead[3] == ArrayWrite[3]) {
        Serial_Printf("\r\n [SUCCESS] W25Q128 Read/Write Test Passed!\r\n");
    } else {
        Serial_Printf("\r\n [ERROR] Data Mismatch! Check SPI Wiring!\r\n");
    }
    Serial_Printf("=========================================\r\n");
    
    while(1)
    {
        // 测试是一次性的，主循环在这里挂起即可
        Delay_ms(1000);
    }
}
