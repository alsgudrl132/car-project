/*
 * delay_us.c
 *
 *  Created on: Jun 16, 2025
 *      Author: psh
 */


#include "delay_us.h"

void delay_us(uint16_t us)
{
	// htim11의 카운터 값을 0으로 만든다.
	__HAL_TIM_SET_COUNTER(&htim11, 0);

	// 가져온 카운터 값이 us(파라미터)보다 작다면 루프
	while((__HAL_TIM_GET_COUNTER(&htim11)) < us);
}
