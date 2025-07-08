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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay_us.h"
#include "motor.h"
#include "ultra_sonic.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
volatile uint8_t btData = 0;
volatile uint16_t speedVal = 700;
volatile uint8_t currentDirection = 'S';
volatile uint8_t isAutoMode = 0;	// 기본값 0 오토모드일경우 1
/* USER CODE END Variables */
/* Definitions for HC_TASK */
osThreadId_t HC_TASKHandle;
const osThreadAttr_t HC_TASK_attributes = {
		.name = "HC_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MOTOR_TASK */
osThreadId_t MOTOR_TASKHandle;
const osThreadAttr_t MOTOR_TASK_attributes = {
		.name = "MOTOR_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for RGB_TASK */
osThreadId_t RGB_TASKHandle;
const osThreadAttr_t RGB_TASK_attributes = {
		.name = "RGB_TASK",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void HC_TASK_F(void *argument);
void MOTOR_TASK_F(void *argument);
void RGB_TASK_F(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of HC_TASK */
	HC_TASKHandle = osThreadNew(HC_TASK_F, NULL, &HC_TASK_attributes);

	/* creation of MOTOR_TASK */
	MOTOR_TASKHandle = osThreadNew(MOTOR_TASK_F, NULL, &MOTOR_TASK_attributes);

	/* creation of RGB_TASK */
	RGB_TASKHandle = osThreadNew(RGB_TASK_F, NULL, &RGB_TASK_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_HC_TASK_F */
/**
 * @brief  Function implementing the HC_TASK thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_HC_TASK_F */
void HC_TASK_F(void *argument)
{
	for(;;)
	{
		HCSR04_Trigger(&sensorLeft);  osDelay(50);
		HCSR04_Trigger(&sensorFront); osDelay(50);
		HCSR04_Trigger(&sensorRight); osDelay(50);

		if(sensorLeft.distance == 0 || sensorLeft.distance > 400) sensorLeft.distance = 400;
		if(sensorFront.distance == 0 || sensorFront.distance > 400) sensorFront.distance = 400;
		if(sensorRight.distance == 0 || sensorRight.distance > 400) sensorRight.distance = 400;

		if(isAutoMode)
		{
			// 1. 정면 장애물 회피
			if(sensorFront.distance < 35)
			{
				sHandler();
				osDelay(100);
				stopHandler();
				osDelay(100);

				if(sensorLeft.distance > sensorRight.distance + 10) {
					aHandler(); // 왼쪽 회피
				}
				else if(sensorRight.distance > sensorLeft.distance + 10) {
					dHandler(); // 오른쪽 회피
				}
				else {
					sHandler(); // 후진
					osDelay(200);
					stopHandler();
					osDelay(100);
					continue;
				}

				osDelay(300);
				stopHandler();
				osDelay(100);
			}
			// 2. 옆면 너무 가까운 경우 → 살짝 꺾기
			else if(sensorLeft.distance < 27)
			{
				// 왼쪽 너무 가까우면 → 오른쪽 살짝 틀기
				wrHandler(); // 전진+우회전
				osDelay(150);
				stopHandler();
				osDelay(50);
			}
			else if(sensorRight.distance < 27)
			{
				// 오른쪽 너무 가까우면 → 왼쪽 살짝 틀기
				wlHandler(); // 전진+좌회전
				osDelay(150);
				stopHandler();
				osDelay(50);
			}
			else
			{
				// 정상 전진
				wHandler();
			}
		}

		osDelay(1);
	}
}


/* USER CODE BEGIN Header_MOTOR_TASK_F */
/**
 * @brief Function implementing the MOTOR_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_MOTOR_TASK_F */
void MOTOR_TASK_F(void *argument)
{
	/* USER CODE BEGIN MOTOR_TASK_F */
	/* Infinite loop */
	for(;;)
	{
		if(!isAutoMode)
		{
			if (btData != 0) // 새로운 명령이 있을 때만 처리
			{
				if (btData >= '0' && btData <= '9')
				{
					speedVal = ((btData - '0') * 70) + 370; // 속도 설정
					// 현재 동작 중인 모터의 속도를 즉시 업데이트
					TIM3->CCR1 = speedVal;
					TIM3->CCR2 = speedVal;
				}
				else {
					currentDirection = btData;
					// 방향 제어 명령들
					switch (currentDirection)
					{
					case 'S':  // Stop
						stopHandler();
						break;
					case 'F':  // Forward
						wHandler();
						break;
					case 'B':  // Backward
						sHandler();
						break;
					case 'R':  // Right (제자리 우회전)
						dHandler();
						break;
					case 'L':  // Left (제자리 좌회전)
						aHandler();
						break;
					case 'G':  // Forward + Left (전진 좌회전)
						wlHandler();
						break;
					case 'H':  // Forward + Right (전진 우회전)
						wrHandler();
						break;
					case 'I':  // Backward + Left (후진 좌회전)
						slHandler();
						break;
					case 'J':  // Backward + Right (후진 우회전)
						srHandler();
						break;
					default:
						// 명령어가 없거나 처리하지 않는 경우 (필요 시 처리)
						break;
					}
				}
				btData = 0;
				osDelay(10);
			}
		}

	}
	/* USER CODE END MOTOR_TASK_F */
}

/* USER CODE BEGIN Header_RGB_TASK_F */
/**
 * @brief Function implementing the RGB_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_RGB_TASK_F */
void RGB_TASK_F(void *argument)
{
	/* USER CODE BEGIN RGB_TASK_F */
	/* Infinite loop */
	for (;;)
	{

		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
		{
			stopHandler();
			isAutoMode = 0;
			osDelay(200);  // 디바운싱
		}
		else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
		{
			isAutoMode = 1;
			osDelay(200);  // 디바운싱
		}

		isAutoMode ? redHandler() : greenHandler();

		osDelay(20);  // 반복 주기
	}
	/* USER CODE END RGB_TASK_F */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

