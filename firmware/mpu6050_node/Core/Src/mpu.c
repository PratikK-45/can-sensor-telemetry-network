#include "mpu.h"

extern I2C_HandleTypeDef hi2c1;

#define MPU6050_ADDR (0x68 << 1)

float Ax, Ay, Az;
float Gx, Gy, Gz;

void MPU6050_Init(void)
{
  uint8_t data;

  data = 0x00;
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x6B, 1, &data, 1, HAL_MAX_DELAY);

  data = 0x00;
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1C, 1, &data, 1, HAL_MAX_DELAY);
  HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1B, 1, &data, 1, HAL_MAX_DELAY);
}

void MPU6050_Read_All(void)
{
  uint8_t Rec_Data[14];
  int16_t Accel_X_RAW;
  int16_t Accel_Y_RAW;
  int16_t Accel_Z_RAW;
  int16_t Gyro_X_RAW;
  int16_t Gyro_Y_RAW;
  int16_t Gyro_Z_RAW;

  HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, 0x3B, 1, Rec_Data, 14, HAL_MAX_DELAY);

  Accel_X_RAW = (Rec_Data[0] << 8 | Rec_Data[1]);
  Accel_Y_RAW = (Rec_Data[2] << 8 | Rec_Data[3]);
  Accel_Z_RAW = (Rec_Data[4] << 8 | Rec_Data[5]);
  Gyro_X_RAW  = (Rec_Data[8] << 8 | Rec_Data[9]);
  Gyro_Y_RAW  = (Rec_Data[10] << 8 | Rec_Data[11]);
  Gyro_Z_RAW  = (Rec_Data[12] << 8 | Rec_Data[13]);

  Ax = (float)Accel_X_RAW / 16384.0f;
  Ay = (float)Accel_Y_RAW / 16384.0f;
  Az = (float)Accel_Z_RAW / 16384.0f;
  Gx = (float)Gyro_X_RAW / 131.0f;
  Gy = (float)Gyro_Y_RAW / 131.0f;
  Gz = (float)Gyro_Z_RAW / 131.0f;
}
