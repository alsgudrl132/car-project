/*
 * buzzer.c
 *
 *  Created on: Jul 9, 2025
 *      Author: psh
 */

#include "buzzer.h"

void startBuzzer()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	osDelay(150);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
	osDelay(100);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
	osDelay(150);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
}

void rejectBuzzer()
{
	for (int i = 0; i < 3; i++)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // 부저 ON
		osDelay(100);  // 짧은 시간
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // 부저 OFF
		osDelay(100);
	}
}


