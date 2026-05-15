#include "stm32f10x.h"                  
#include "Delay.h"
#include "AD.h"
#include "Serial.h"   

#define ADC_FILTER_TIMES 20 

int main(void) {
    AD_Init();        
    Serial_Init();    
    
    Serial_Printf("==================================\r\n");
    Serial_Printf("   Battery Voltage Monitor V1.0   \r\n");
    Serial_Printf("==================================\r\n");

    while(1) {
        uint32_t ad_sum = 0; 
        for(int i = 0; i < ADC_FILTER_TIMES; i++) {
            ad_sum += AD_GetValue();
            Delay_ms(1); 
        }
        uint16_t ad_value_avg = ad_sum / ADC_FILTER_TIMES; 
        
        float pin_voltage = (float)ad_value_avg / 4095.0f * 3.3f; 
		float battery_voltage = pin_voltage * 5.563f;
        
        Serial_Printf("AD: %4d  |  Pin V: %.2f V  |  BAT V: %.2f V\r\n", 
                      ad_value_avg, pin_voltage, battery_voltage);
        
        Delay_ms(500); 
    }
}
