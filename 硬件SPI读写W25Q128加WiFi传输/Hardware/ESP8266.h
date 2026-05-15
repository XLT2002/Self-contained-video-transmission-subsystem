#ifndef __ESP8266_H
#define __ESP8266_H

void ESP8266_Init_Server(void);
uint8_t ESP8266_CheckDownloadCommand(void);
void ESP8266_Send_TCP_Data(uint8_t *data, uint16_t len);

#endif
