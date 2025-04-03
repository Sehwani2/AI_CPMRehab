/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : rs485.h
  * @brief          : Header for rs485.c file.
  *                   This file contains RS485 communication functions.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __SERVO_H
#define __SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Function Prototypes */
void RS485_MotorInit(UART_HandleTypeDef *huart);
void RS485_SendCommand(const char *cmd);
void RS485_MotorEnable(void);
void RS485_MotorDisable(void);



#ifdef __cplusplus
}
#endif

#endif /* __SERVO_H */
