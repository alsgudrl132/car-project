/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
#include "delay_us.h"
#include "motor.h"
#include "ultra_sonic.h"
#include "usart.h"
#include "string.h"
#include "rc522.h"
#include "buzzer.h"

/* Private variables ---------------------------------------------------------*/
volatile uint8_t btData = 0;
volatile uint16_t speedVal = 700;
volatile uint8_t currentDirection = 'S';
volatile uint8_t isAutoMode = 0;
volatile uint8_t status;
volatile uint8_t str[MAX_LEN]; // Max_LEN = 16
volatile uint8_t sNum[5];
volatile uint8_t isEngineOn = 0;

/* Definitions for RTOS threads */
osThreadId_t HC_TASKHandle;
const osThreadAttr_t HC_TASK_attributes = {
		.name = "HC_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t MOTOR_TASKHandle;
const osThreadAttr_t MOTOR_TASK_attributes = {
		.name = "MOTOR_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t RGB_TASKHandle;
const osThreadAttr_t RGB_TASK_attributes = {
		.name = "RGB_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t RFID_TASKHandle;
const osThreadAttr_t RFID_TASK_attributes = {
		.name = "RFID_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};

/* Function prototypes */
void HC_TASK_F(void *argument);
void MOTOR_TASK_F(void *argument);
void RGB_TASK_F(void *argument);
void RFID_TASK_F(void *argument);

void MX_FREERTOS_Init(void)
{
	HC_TASKHandle = osThreadNew(HC_TASK_F, NULL, &HC_TASK_attributes);
	MOTOR_TASKHandle = osThreadNew(MOTOR_TASK_F, NULL, &MOTOR_TASK_attributes);
	RGB_TASKHandle = osThreadNew(RGB_TASK_F, NULL, &RGB_TASK_attributes);
	RFID_TASKHandle = osThreadNew(RFID_TASK_F, NULL, &RFID_TASK_attributes);
}

void HC_TASK_F(void *argument)
{
	for (;;)
	{
		if(isEngineOn)
		{
			HCSR04_Trigger(&sensorLeft);  osDelay(50);
			HCSR04_Trigger(&sensorFront); osDelay(50);
			HCSR04_Trigger(&sensorRight); osDelay(50);

			if (sensorLeft.distance == 0 || sensorLeft.distance > 400)   sensorLeft.distance = 400;
			if (sensorFront.distance == 0 || sensorFront.distance > 400) sensorFront.distance = 400;
			if (sensorRight.distance == 0 || sensorRight.distance > 400) sensorRight.distance = 400;

			if (isAutoMode)
			{
				if (sensorFront.distance < 33)
				{
					sHandler();
					osDelay(100);
					stopHandler();
					osDelay(100);

					if (sensorLeft.distance > sensorRight.distance + 10)
					{
						aHandler();
					}
					else if (sensorRight.distance > sensorLeft.distance + 10)
					{
						dHandler();
					}
					else
					{
						sHandler();
						osDelay(200);
						stopHandler();
						osDelay(100);
						continue;
					}

					osDelay(300);
					stopHandler();
					osDelay(100);
				}
				else if (sensorLeft.distance < 24)
				{
					wrHandler();
					osDelay(150);
					stopHandler();
					osDelay(50);
				}
				else if (sensorRight.distance < 24)
				{
					wlHandler();
					osDelay(150);
					stopHandler();
					osDelay(50);
				}
				else
				{
					wHandler();
				}
			}
		}


		osDelay(1);
	}
}

void MOTOR_TASK_F(void *argument)
{
	for(;;)
	{
		if (!isEngineOn)
		{
			if (btData != 0)
			{
				rejectBuzzer();  // 시동 안 걸렸는데 블루투스 제어 시도
				btData = 0;      // 명령 무시
			}
			osDelay(10);
			continue;
		}
		if(!isAutoMode)
		{
			if (btData != 0)
			{
				if (btData >= '0' && btData <= '9')
				{
					speedVal = ((btData - '0') * 70) + 370;
					TIM3->CCR1 = speedVal;
					TIM3->CCR2 = speedVal;
				}
				else {
					currentDirection = btData;
					switch (currentDirection)
					{
					case 'S': stopHandler(); break;
					case 'F': wHandler(); break;
					case 'B': sHandler(); break;
					case 'R': dHandler(); break;
					case 'L': aHandler(); break;
					case 'G': wlHandler(); break;
					case 'H': wrHandler(); break;
					case 'I': slHandler(); break;
					case 'J': srHandler(); break;
					default: break;
					}
				}
				btData = 0;
				osDelay(10);
			}
		}
	}
}

void RGB_TASK_F(void *argument)
{
	for (;;)
	{
		if (!isEngineOn)
		{
			if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET ||
					HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
			{
				rejectBuzzer(); // 시동 안 켰는데 버튼 눌림
				osDelay(200);   // 디바운싱
			}
			continue; // 더 이상 동작 못하게
		}

		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
		{
			stopHandler();
			isAutoMode = 0;
			osDelay(200);
		}
		else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
		{
			isAutoMode = 1;
			osDelay(200);
		}

		isAutoMode ? redHandler() : greenHandler();
		osDelay(20);
	}

}

void RFID_TASK_F(void *argument)
{
	uint8_t masterCardUID[4] = {225, 157, 194, 1};

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

	for(;;)
	{
		if (MFRC522_Request(PICC_REQIDL, str) == MI_OK)
		{
			if (MFRC522_Anticoll(str) == MI_OK)
			{
				if ((isEngineOn == 0) &&
						(str[0] == masterCardUID[0] && str[1] == masterCardUID[1] &&
								str[2] == masterCardUID[2] && str[3] == masterCardUID[3]))
				{
					isEngineOn = 1;

					startBuzzer();
				}
			}
		}

		isEngineOn ? HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, 1) : HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, 0);
		osDelay(200);
	}
}


/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

