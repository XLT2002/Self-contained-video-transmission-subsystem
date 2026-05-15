#include "stm32f10x.h"                  // Device header


/**
  * @brief  将真实角度转换为 MSP 协议的底层十六进制数据包
  * @param  roll/pitch/yaw: 真实的欧拉角 (例如 15.5 度)
  * @param  tx_buffer: 用于存放生成数据的数组指针
  * @retval 数据包总长度 (字节)
  */
uint8_t MSP_PackAttitude(float roll, float pitch, float yaw, uint8_t *tx_buffer) {
    uint8_t checksum = 0;
    
    // 1. 数据缩放：大疆 MSP 协议要求角度必须放大 10 倍并转为整数
    // 例如：15.5度 -> 155
    int16_t r = (int16_t)(roll * 10.0f);   
    int16_t p = (int16_t)(pitch * 10.0f);
    int16_t y = (int16_t)(yaw * 10.0f);

    // 2. 组装包头
    tx_buffer[0] = '$';
    tx_buffer[1] = 'M';
    tx_buffer[2] = '>';
    
    // 3. 组装长度
    tx_buffer[3] = 6;  
    checksum ^= 6;
    
    // 4. 组装命令字 (108 = MSP_ATTITUDE)
    tx_buffer[4] = 108; 
    checksum ^= 108;

    // 5. 组装数据负载 (低位在前，高位在后 - 小端序)
    tx_buffer[5] = r & 0xFF;         checksum ^= tx_buffer[5];
    tx_buffer[6] = (r >> 8) & 0xFF;  checksum ^= tx_buffer[6];
    
    tx_buffer[7] = p & 0xFF;         checksum ^= tx_buffer[7];
    tx_buffer[8] = (p >> 8) & 0xFF;  checksum ^= tx_buffer[8];
    
    tx_buffer[9] = y & 0xFF;         checksum ^= tx_buffer[9];
    tx_buffer[10] = (y >> 8) & 0xFF; checksum ^= tx_buffer[10];

    // 6. 组装校验和
    tx_buffer[11] = checksum;

    // 7. 返回这个包的总长度（12个字节）
    return 12; 
}
