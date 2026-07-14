#include "ds18b20.h"

extern TIM_HandleTypeDef htim2;

#define DS18B20_PORT GPIOA
#define DS18B20_PIN  GPIO_PIN_1

float DS18B20_Temp = 0.0f;
static uint8_t conversion_started = 0;
static uint32_t conversion_tick = 0;

static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

static void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void delay_us(uint16_t us)
{
  __HAL_TIM_SET_COUNTER(&htim2, 0);
  while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

uint8_t DS18B20_Start(void)
{
  uint8_t Response = 0;

  Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
  HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
  delay_us(480);

  Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
  delay_us(80);

  if (!(HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)))
  {
    Response = 1;
  }
  delay_us(400);
  return Response;
}

void DS18B20_Write(uint8_t data)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    if (data & (1 << i))
    {
      delay_us(2);
      Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
      delay_us(60);
    }
    else
    {
      delay_us(60);
      Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
      delay_us(2);
    }
  }
}

uint8_t DS18B20_Read(void)
{
  uint8_t value = 0;

  for (uint8_t i = 0; i < 8; i++)
  {
    Set_Pin_Output(DS18B20_PORT, DS18B20_PIN);
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(2);
    Set_Pin_Input(DS18B20_PORT, DS18B20_PIN);
    delay_us(8);
    if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN))
    {
      value |= (1 << i);
    }
    delay_us(60);
  }
  return value;
}

float DS18B20_GetTemp(void)
{
  uint8_t Temp_L;
  uint8_t Temp_H;
  int16_t TEMP;

  if (conversion_started == 0)
  {
    if (DS18B20_Start())
    {
      DS18B20_Write(0xCC);
      DS18B20_Write(0x44);
      conversion_tick = HAL_GetTick();
      conversion_started = 1;
    }
    return DS18B20_Temp;
  }

  if ((HAL_GetTick() - conversion_tick) < 750)
  {
    return DS18B20_Temp;
  }

  if (!DS18B20_Start())
  {
    conversion_started = 0;
    return DS18B20_Temp;
  }

  DS18B20_Write(0xCC);
  DS18B20_Write(0xBE);
  Temp_L = DS18B20_Read();
  Temp_H = DS18B20_Read();
  TEMP = (Temp_H << 8) | Temp_L;
  DS18B20_Temp = (float)TEMP / 16.0f;

  if (DS18B20_Start())
  {
    DS18B20_Write(0xCC);
    DS18B20_Write(0x44);
    conversion_tick = HAL_GetTick();
    conversion_started = 1;
  }
  else
  {
    conversion_started = 0;
  }

  return DS18B20_Temp;
}
