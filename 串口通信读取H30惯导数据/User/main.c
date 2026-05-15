#include "stm32f10x.h"                  
#include "Delay.h"
#include "usart.h"
#include <stdio.h>

int main(void)
{
    // 初始化中断优先级分组[cite: 21]
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化串口1，波特率必须与H30模块一致 (460800)[cite: 21]
    H30_UART1_Init(460800); 

    // 打印监控台表头
    printf("\r\n======================================================\r\n");
    printf("   WHEELTEC H30 IMU Sensor Fusion Monitor V1.0   \r\n");
    printf("======================================================\r\n");

    while(1)
    {
        // ================= 核心姿态解算数据 (欧拉角) =================
        // %7.2f 表示总长度7位，保留2位小数，能让数据像表格一样极其整齐对齐
        printf("[Euler] Pitch: %7.2f | Roll: %7.2f | Yaw: %7.2f\r\n", Pitch, Roll, Yaw);
        
        // ================= 加速度数据 (如果需要观测，取消注释即可) =================
        printf("[Acc]   X: %7.2f | Y: %7.2f | Z: %7.2f\r\n", AccX, AccY, AccZ);
        
        // ================= 角速度数据 (如果需要观测，取消注释即可) =================
        printf("[Gyro]  X: %7.2f | Y: %7.2f | Z: %7.2f\r\n", GyroX, GyroY, GyroZ);

        // 刷新率控制在 10Hz (100ms)，避免电脑屏幕刷新太快看不清[cite: 21]
        Delay_ms(100); 
    }
}
