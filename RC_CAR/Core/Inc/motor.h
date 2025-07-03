/*
 * motor.h
 *
 * Created on: Jul 3, 2025
 * Author: psh
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"

// 기본 모터 제어 함수들
void stopHandler(void);
void wHandler(void);      // 전진
void sHandler(void);      // 후진
void aHandler(void);      // 좌회전 (제자리)
void dHandler(void);      // 우회전 (제자리)

// 부드러운 커브 제어 함수들
void wlHandler(void);     // 전진 + 좌회전
void wrHandler(void);     // 전진 + 우회전
void slHandler(void);     // 후진 + 좌회전
void srHandler(void);     // 후진 + 우회전

// 속도 초기화 함수
void speedInit(void);

#endif /* INC_MOTOR_H_ */
