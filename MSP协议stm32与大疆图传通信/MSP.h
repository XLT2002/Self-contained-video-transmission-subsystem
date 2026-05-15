#ifndef __MSP_H
#define __MSP_H

// 声明一个专门用来打包姿态数据的函数
// 参数：roll, pitch, yaw (传入真实物理角度)
// 参数：tx_buffer (传入一个空数组，函数会把打包好的十六进制代码填进去)
// 返回值：打包好的数据总长度
uint8_t MSP_PackAttitude(float roll, float pitch, float yaw, uint8_t *tx_buffer);

#endif
