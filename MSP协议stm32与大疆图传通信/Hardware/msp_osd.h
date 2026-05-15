#ifndef __MSP_OSD_H
#define __MSP_OSD_H

#include "stm32f10x.h"

void MSP_ProcessRx(uint8_t c);
uint8_t MSP_CheckAndReply(void); // 【核心新增】主循环轮询接口

void OSD_Heartbeat(void); 
void OSD_Print(uint8_t row, uint8_t col, const char *str);
void OSD_Update(void);
void OSD_Clear(void);

#endif
