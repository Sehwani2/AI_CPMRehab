/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : encoder.h
  * @brief          : Header for encoder.c file.
  *                   This file contains encoder definitions and functions.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __ENCODER_H
#define __ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>

/* Encoder Definitions */
#define MAX_ENCODER_COUNT 19999

/* Function Prototypes */
void Encoder_SetTimHandle(TIM_HandleTypeDef *htim);
float Encoder_GetAngle(void);
void Encoder_Start(void);
void Encoder_Init(void);
int Encoder_GetAngleInt(void);

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H */
