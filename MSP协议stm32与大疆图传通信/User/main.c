#include "stm32f10x.h"
#include "uart.h"
#include "msp_osd.h"
#include "Delay.h"
#include "AD.h"
#include "usart.h"
#include <stdio.h>

#define VOLTAGE_DIVIDER_RATIO  5.545f

int main(void) {
    // 1. 中断优先级分组
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 

    // 2. 初始化核心外设
    AD_Init();                  
    H30_UART1_Init(460800); 
    USART2_Init();          
    
    char buf[32];
    uint16_t dji_heartbeat_count = 0; // 大疆心跳计步器
    uint8_t osd_init_done = 0;        // OSD 画布初始化标志
    uint8_t osd_update_counter = 0;   // 画面刷新分频器

    // 【主循环】：全速奔跑，没有任何 Delay_ms 阻塞！
    while(1) {
        
        // 1. 全速处理 FIFO 队列里的命令（瞬间完成，绝无溢出！）
        uint8_t processed_cmd = MSP_CheckAndReply();
        
        // 2. 捕捉到大疆的 101 心跳！
        if (processed_cmd == 101) {
            
            // ============== 阶段 A：开机缓冲期 ==============
            if (osd_init_done == 0) {
                dji_heartbeat_count++;
                
                // 大疆发 50 次 101 约等于 1 秒钟
                // 利用这个时间，确保大疆已经安全收到了我们底层的 189 画布回复
                if (dji_heartbeat_count > 50) { 
                    OSD_Clear();       // 第 1 秒结束，清屏
                    osd_init_done = 1; // 宣告缓冲期结束，进入正常画图
                }
            } 
            // ============== 阶段 B：稳定渲染期 ==============
            else {
                // 降频发送：每收到 2 次心跳，发一帧 OSD 数据 (约 20Hz)
                osd_update_counter++;
                if (osd_update_counter >= 2) {
                    osd_update_counter = 0;

                    // 1. 喂画布狗，防止大疆强行关闭引擎
                    OSD_Heartbeat(); 

                    // 2. 打印电压
                    uint16_t ad_val = AD_GetValue();
                    float voltage = ((float)ad_val / 4096.0f) * 3.3f * VOLTAGE_DIVIDER_RATIO;
                    sprintf(buf, "VBAT: %.2fV", voltage);
                    OSD_Print(2, 2, buf);
                    
                    // 3. 打印姿态
                    sprintf(buf, "P: %.1f R: %.1f", Pitch, Roll);
                    OSD_Print(8, 15, buf);
                    
                    sprintf(buf, "YAW: %.1f", Yaw);
                    OSD_Print(10, 15, buf);
                    
                    // 4. 提交渲染
                    OSD_Update(); 
                }
            }
        }
    }
}
