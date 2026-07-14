#ifndef MPU_H
#define MPU_H

#include "main.h"

#define MPU6050_ADDR (0x68 << 1)

extern float Ax, Ay, Az;
extern float Gx, Gy, Gz;

void MPU6050_Init(void);
void MPU6050_Read_All(void);

#endif
