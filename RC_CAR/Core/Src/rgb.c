/*
 * rgb.c
 *
 *  Created on: Jul 4, 2025
 *      Author: psh
 */

#include "rgb.h"

void redHandler()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
}

void greenHandler()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
}

void blueHandler()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
}



