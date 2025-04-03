/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : encoder.c
  * @brief          : Implementation of encoder functions
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "encoder.h"

/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef *_htim;
static uint32_t initial_encoder_count = 0;  // 초기 엔코더 위치
static uint32_t current_encoder_count = 0;  // 현재 엔코더 카운트 값

/* Function implementations --------------------------------------------------*/
/**
  * @brief 사용할 타이머 핸들을 설정
  * @param htim 타이머 핸들 포인터
  * @retval None
  */
void Encoder_SetTimHandle(TIM_HandleTypeDef *htim)
{
  _htim = htim;
}

/**
  * @brief 엔코더 초기화
  * @retval None
  */
void Encoder_Init(void)
{
  initial_encoder_count = __HAL_TIM_GET_COUNTER(_htim);
}


/**
  * @brief 엔코더 시작
  * @retval None
  */
void Encoder_Start(void)
{
  HAL_TIM_Encoder_Start(_htim, TIM_CHANNEL_ALL);
  Encoder_Init();  // 초기 카운트 저장
}

/**
  * @brief 엔코더 카운트 값을 360도로 변환하는 함수
  * @retval 계산된 각도 (0-360도)
  */
float Encoder_GetAngle(void)
{
  // 현재 엔코더 카운트 값 가져오기
  current_encoder_count = __HAL_TIM_GET_COUNTER(_htim);

  // 카운트 차이 계산
  int32_t count_diff = current_encoder_count - initial_encoder_count;

  // 0에서 360도 범위로 변환
  // 카운트 차이를 MAX_ENCODER_COUNT로 나누어 360도를 곱함
  float angle = (float)count_diff / (float)MAX_ENCODER_COUNT * 360.0f;

  // 부호에 따라 범위 조정
//  if (angle < 0)
//  {
//    angle += 360.0f;  // 360도 내로 값을 맞추기 위해 양수로 처리
//  }

  return angle;
}

int Encoder_GetAngleInt(void)
{
    // float 타입의 각도 값 가져오기
    float angle = Encoder_GetAngle();

    // 반올림하여 정수로 변환 (0.5를 더해 소수점 반올림 효과)
    int intAngle = (int)angle;

    // 360도 범위 내로 유지 (0-359)
    intAngle = intAngle % 360;
    if (intAngle < 0)
        intAngle += 360;

    return intAngle;
}
