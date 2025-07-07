#ifndef INC_ULTRA_SONIC_H_
#define INC_ULTRA_SONIC_H_

#include "main.h"
#include "tim.h"

typedef struct {
  GPIO_TypeDef* trigPort;
  uint16_t trigPin;
  TIM_HandleTypeDef* htim;
  uint32_t channel;

  volatile uint16_t IC_Value1;
  volatile uint16_t IC_Value2;
  volatile uint16_t echoTime;
  volatile uint8_t captureFlag;
  volatile uint8_t distance;
} HCSR04_t;

extern HCSR04_t sensorLeft;
extern HCSR04_t sensorFront;
extern HCSR04_t sensorRight;

void HCSR04_Init(void);
void HCSR04_Trigger(HCSR04_t* sensor);
void HCSR04_IC_CaptureCallback(TIM_HandleTypeDef* htim);

#endif /* INC_ULTRA_SONIC_H_ */
