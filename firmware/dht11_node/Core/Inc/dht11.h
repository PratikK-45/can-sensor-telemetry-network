#ifndef DHT11_H
#define DHT11_H

#include "main.h"

void DHT11_Start(void);
uint8_t DHT11_Check_Response(void);
uint8_t DHT11_Read(void);
void DHT11_Read_Data(void);

extern float DHT_Temperature;
extern float DHT_Humidity;

#endif
