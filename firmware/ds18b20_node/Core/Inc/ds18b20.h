#ifndef DS18B20_H
#define DS18B20_H

#include "main.h"

extern float DS18B20_Temp;

void delay_us(uint16_t us);
uint8_t DS18B20_Start(void);
void DS18B20_Write(uint8_t data);
uint8_t DS18B20_Read(void);
float DS18B20_GetTemp(void);

#endif
