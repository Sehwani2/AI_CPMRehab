/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : rs485.c
  * @brief          : Implementation of RS485 communication functions
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <servo.h>
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef *_huart;

/* Function implementations --------------------------------------------------*/
/**
  * @brief 서보 모터 초기화 및 설정
  * @param huart UART 핸들 포인터
  * @retval None
  */
void RS485_MotorInit(UART_HandleTypeDef *huart)
{
    _huart = huart;
}

/**
  * @brief 모터 활성화
  * @retval None
  */
void RS485_MotorEnable(void)
{
    RS485_SendCommand("ME");    // 모터 활성화
    HAL_Delay(500);             // 모터 활성화 대기
}

/**
  * @brief 모터 비활성화
  * @retval None
  */
void RS485_MotorDisable(void)
{
  RS485_SendCommand("ST");
  RS485_SendCommand("MD");    // 모터 비활성화
}

/**
  * @brief RS485를 통해 명령 전송
  * @param cmd 전송할 명령
  * @retval None
  */
void RS485_SendCommand(const char *cmd)
{
    char buffer[32];
    sprintf(buffer, "%s\r", cmd);
    HAL_UART_Transmit(_huart, (uint8_t*)buffer, strlen(buffer), 100);
    HAL_Delay(20);
}
