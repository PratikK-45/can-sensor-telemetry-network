/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"
#include <stdio.h>
#include <string.h>

CAN_HandleTypeDef hcan1;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
CAN_RxHeaderTypeDef RxHeader;
CAN_FilterTypeDef canfilter;
uint8_t RxData[8];

float Ax, Ay, Az;
float Gx, Gy, Gz;
float DS_Temp;
int Speed;
uint8_t Temp;
uint8_t Hum;

uint8_t MPU_Fault = 1;
uint8_t DHT_Fault = 1;
uint8_t DS_Fault = 1;
uint8_t Speed_Fault = 1;

uint32_t MPU_LastRx = 0;
uint32_t DHT_LastRx = 0;
uint32_t DS_LastRx = 0;
uint32_t Speed_LastRx = 0;

char msg[300];
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void CAN_Filter_Config(void);
/* USER CODE END PFP */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  CAN_Filter_Config();
  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  sprintf(msg, "CAN Receiver Ready\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
  /* USER CODE END 2 */

  while (1)
  {
    uint32_t now = HAL_GetTick();
    MPU_Fault = ((now - MPU_LastRx) > 1000);
    DHT_Fault = ((now - DHT_LastRx) > 1000);
    DS_Fault = ((now - DS_LastRx) > 1000);
    Speed_Fault = ((now - Speed_LastRx) > 1000);

    snprintf(msg, sizeof(msg),
      "AX=%.2f AY=%.2f AZ=%.2f\r\n"
      "GX=%.2f GY=%.2f GZ=%.2f\r\n"
      "MPU6050=%s\r\n"
      "TEMP=%d C HUM=%d %% DHT=%s\r\n"
      "DS18B20=%.2f C DS=%s\r\n"
      "Speed=%d km/h SpeedECU=%s\r\n\r\n",
      Ax, Ay, Az,
      Gx, Gy, Gz,
      MPU_Fault ? "FAULT" : "OK",
      Temp, Hum,
      DHT_Fault ? "FAULT" : "OK",
      DS_Temp,
      DS_Fault ? "FAULT" : "OK",
      Speed,
      Speed_Fault ? "FAULT" : "OK");

    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    HAL_Delay(500);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                               | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_CAN1_Init(void)
{
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_11TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
void CAN_Filter_Config(void)
{
  canfilter.FilterActivation = ENABLE;
  canfilter.FilterBank = 0;
  canfilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  canfilter.FilterIdHigh = 0;
  canfilter.FilterIdLow = 0;
  canfilter.FilterMaskIdHigh = 0;
  canfilter.FilterMaskIdLow = 0;
  canfilter.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilter.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilter.SlaveStartFilterBank = 14;
  if (HAL_CAN_ConfigFilter(&hcan1, &canfilter) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  int16_t temp1, temp2, temp3;
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);

  switch (RxHeader.StdId)
  {
    /* MPU6050 Accelerometer */
    case 0x100:
      temp1 = (int16_t)((RxData[0] << 8) | RxData[1]);
      temp2 = (int16_t)((RxData[2] << 8) | RxData[3]);
      temp3 = (int16_t)((RxData[4] << 8) | RxData[5]);
      Ax = (float)temp1 / 100.0f;
      Ay = (float)temp2 / 100.0f;
      Az = (float)temp3 / 100.0f;
      MPU_LastRx = HAL_GetTick();
      break;

    /* MPU6050 Gyroscope */
    case 0x101:
      temp1 = (int16_t)((RxData[0] << 8) | RxData[1]);
      temp2 = (int16_t)((RxData[2] << 8) | RxData[3]);
      temp3 = (int16_t)((RxData[4] << 8) | RxData[5]);
      Gx = (float)temp1 / 100.0f;
      Gy = (float)temp2 / 100.0f;
      Gz = (float)temp3 / 100.0f;
      MPU_LastRx = HAL_GetTick();
      break;

    /* DHT11 */
    case 0x201:
      Temp = RxData[0];
      Hum = RxData[1];
      DHT_LastRx = HAL_GetTick();
      break;

    /* DS18B20 */
    case 0x301:
      temp1 = (int16_t)((RxData[0] << 8) | RxData[1]);
      DS_Temp = (float)temp1 / 100.0f;
      DS_LastRx = HAL_GetTick();
      break;

    /* Speed */
    case 0x401:
      Speed = (int16_t)((RxData[0] << 8) | RxData[1]);
      Speed_LastRx = HAL_GetTick();
      break;

    /* Unknown CAN ID */
    default:
      sprintf(msg, "Unknown ID: 0x%03lX\r\n", RxHeader.StdId);
      HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
      break;
  }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
