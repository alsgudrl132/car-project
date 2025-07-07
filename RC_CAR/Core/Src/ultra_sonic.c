#include "ultra_sonic.h"
#include "delay_us.h"

HCSR04_t sensorLeft = {
  .trigPort = GPIOC, .trigPin = GPIO_PIN_9,
  .htim = &htim1, .channel = TIM_CHANNEL_1
};
HCSR04_t sensorFront = {
  .trigPort = GPIOB, .trigPin = GPIO_PIN_8,
  .htim = &htim1, .channel = TIM_CHANNEL_2
};
HCSR04_t sensorRight = {
  .trigPort = GPIOB, .trigPin = GPIO_PIN_9,
  .htim = &htim1, .channel = TIM_CHANNEL_3
};

void HCSR04_Init(void)
{
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
}

void HCSR04_Trigger(HCSR04_t* sensor)
{
  HAL_GPIO_WritePin(sensor->trigPort, sensor->trigPin, GPIO_PIN_RESET);
  delay_us(2);
  HAL_GPIO_WritePin(sensor->trigPort, sensor->trigPin, GPIO_PIN_SET);
  delay_us(10);
  HAL_GPIO_WritePin(sensor->trigPort, sensor->trigPin, GPIO_PIN_RESET);

  sensor->captureFlag = 0;

  __HAL_TIM_SET_CAPTUREPOLARITY(sensor->htim, sensor->channel, TIM_INPUTCHANNELPOLARITY_RISING);
  // Correctly calculate the interrupt enable bit for the channel
  uint32_t ch_bit = (TIM_IT_CC1 << (sensor->channel / 4));
  __HAL_TIM_ENABLE_IT(sensor->htim, ch_bit);
}

void HCSR04_IC_CaptureCallback(TIM_HandleTypeDef* htim)
{
  HCSR04_t* sensor = NULL;

  if (htim->Instance == TIM1) {
    // Use htim->Channel to identify the active channel, which is set by the HAL IRQ handler.
    switch (htim->Channel)
    {
      case HAL_TIM_ACTIVE_CHANNEL_1:
        sensor = &sensorLeft;
        break;
      case HAL_TIM_ACTIVE_CHANNEL_2:
        sensor = &sensorFront;
        break;
      case HAL_TIM_ACTIVE_CHANNEL_3:
        sensor = &sensorRight;
        break;
      default:
        return;
    }

    if (sensor->captureFlag == 0) {
      // First capture (rising edge)
      sensor->IC_Value1 = HAL_TIM_ReadCapturedValue(sensor->htim, sensor->channel);
      sensor->captureFlag = 1;
      __HAL_TIM_SET_CAPTUREPOLARITY(sensor->htim, sensor->channel, TIM_INPUTCHANNELPOLARITY_FALLING);
    } else {
      // Second capture (falling edge)
      sensor->IC_Value2 = HAL_TIM_ReadCapturedValue(sensor->htim, sensor->channel);
      __HAL_TIM_SET_COUNTER(sensor->htim, 0);

      if (sensor->IC_Value2 > sensor->IC_Value1) {
        sensor->echoTime = sensor->IC_Value2 - sensor->IC_Value1;
      } else {
        sensor->echoTime = (0xFFFF - sensor->IC_Value1) + sensor->IC_Value2;
      }

      sensor->distance = sensor->echoTime / 58;

      sensor->captureFlag = 0;
      __HAL_TIM_SET_CAPTUREPOLARITY(sensor->htim, sensor->channel, TIM_INPUTCHANNELPOLARITY_RISING);
      
      // Correctly calculate the interrupt disable bit for the channel
      uint32_t ch_bit = (TIM_IT_CC1 << (sensor->channel / 4));
      __HAL_TIM_DISABLE_IT(sensor->htim, ch_bit);
    }
  }
}
