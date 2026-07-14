#include "dht11.h"

extern TIM_HandleTypeDef htim2;

#define DHT11_PORT GPIOA
#define DHT11_PIN  GPIO_PIN_1

float DHT_Temperature = 0;
float DHT_Humidity = 0;

static void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
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

void DHT11_Start(void)
{
  Set_Pin_Output(DHT11_PORT, DHT11_PIN);
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);
  HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
  delay_us(30);
  Set_Pin_Input(DHT11_PORT, DHT11_PIN);
}

uint8_t DHT11_Check_Response(void)
{
  uint8_t Response = 0;
  uint32_t timeout = 10000;

  delay_us(40);
  if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
  {
    delay_us(80);
    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
    {
      Response = 1;
    }
  }

  timeout = 10000;
  while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
  {
    if (timeout-- == 0)
    {
      return 0;
    }
  }
  return Response;
}

uint8_t DHT11_Read(void)
{
  uint8_t i = 0;
  uint8_t j;
  uint32_t timeout;

  for (j = 0; j < 8; j++)
  {
    timeout = 10000;
    while (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
    {
      if (timeout-- == 0)
      {
        return 0;
      }
    }
    delay_us(40);
    if (!(HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)))
    {
      i &= ~(1 << (7 - j));
    }
    else
    {
      i |= (1 << (7 - j));
      timeout = 10000;
      while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
      {
        if (timeout-- == 0)
        {
          return 0;
        }
      }
    }
  }
  return i;
}

void DHT11_Read_Data(void)
{
  DHT_Temperature = 0;
  DHT_Humidity = 0;
  uint8_t Rh_byte1;
  uint8_t Rh_byte2;
  uint8_t Temp_byte1;
  uint8_t Temp_byte2;
  uint8_t SUM;

  DHT11_Start();
  if (DHT11_Check_Response())
  {
    Rh_byte1 = DHT11_Read();
    Rh_byte2 = DHT11_Read();
    Temp_byte1 = DHT11_Read();
    Temp_byte2 = DHT11_Read();
    SUM = DHT11_Read();

    if (SUM == (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2))
    {
      DHT_Humidity = (float)Rh_byte1;
      DHT_Temperature = (float)Temp_byte1;
    }
  }
}
