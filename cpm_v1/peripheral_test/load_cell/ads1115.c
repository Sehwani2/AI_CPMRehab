/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ads1115.c
  * @brief          : Implementation of ADS1115 ADC functions
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ads1115.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static I2C_HandleTypeDef *_hi2c;
static int16_t adc_offset = 0;         // 영점 오프셋 값
static float scale_factor = 1.0f;      // 스케일 팩터 (무게 단위 변환용)
static bool is_calibrated = false;     // 캘리브레이션 상태

/* Function implementations --------------------------------------------------*/
/**
  * @brief 사용할 I2C 핸들을 설정
  * @param hi2c I2C 핸들 포인터
  * @retval None
  */
void ADS1115_SetI2CHandle(I2C_HandleTypeDef *hi2c)
{
  _hi2c = hi2c;
}

/**
  * @brief ADS1115 초기화 함수
  * @retval HAL 상태
  */
HAL_StatusTypeDef ADS1115_Init(void) {
  uint8_t buffer[3];
  HAL_StatusTypeDef status;

  // 포인터 레지스터를 Config 레지스터로 설정
  buffer[0] = ADS1115_REG_CONFIG;

  // ADS1115 설정:
  // - 단일 변환
  // - 차동 입력 A0-A1
  // - PGA 게인 = 4 (±1.024V 범위, 필요에 따라 조정)
  // - 단일 샷 모드
  // - 128 SPS 데이터 레이트
  // - 비교기 비활성화
  uint16_t config = ADS1115_OS_SINGLE | ADS1115_MUX_SINGLE_0 |
                   ADS1115_PGA_4_096V | ADS1115_MODE_CONT |
                   ADS1115_DR_128SPS | ADS1115_COMP_QUE_DIS;

  buffer[1] = (uint8_t)(config >> 8);    // MSB
  buffer[2] = (uint8_t)(config & 0xFF);  // LSB

  // 설정값을 ADS1115에 쓰기
  status = HAL_I2C_Master_Transmit(_hi2c, ADS1115_I2C_ADDR << 1, buffer, 3, HAL_MAX_DELAY);

  // 설정 확인
  if(status == HAL_OK) {
    uint8_t check_reg = ADS1115_REG_CONFIG;
    uint8_t check_data[2];

    HAL_I2C_Master_Transmit(_hi2c, ADS1115_I2C_ADDR << 1, &check_reg, 1, 100);
    HAL_I2C_Master_Receive(_hi2c, ADS1115_I2C_ADDR << 1, check_data, 2, 100);

    printf("Configuration check: 0x%02X%02X\r\n", check_data[0], check_data[1]);
  }

  return status;
}

/**
  * @brief ADS1115에서 ADC 값 읽기
  * @retval 16비트 ADC 값
  */
int16_t ADS1115_ReadADC(void) {
  uint8_t buffer[3];
  int16_t adc_value = 0;

  // 포인터 레지스터를 Config 레지스터로 설정
  buffer[0] = ADS1115_REG_CONFIG;

  // ADS1115 설정 및 변환 시작
  uint16_t config = ADS1115_OS_SINGLE | ADS1115_MUX_SINGLE_0 |
                   ADS1115_PGA_4_096V | ADS1115_MODE_CONT |
                   ADS1115_DR_128SPS | ADS1115_COMP_QUE_DIS;

  buffer[1] = (uint8_t)(config >> 8);    // MSB
  buffer[2] = (uint8_t)(config & 0xFF);  // LSB

  // 설정값을 ADS1115에 쓰기
  HAL_I2C_Master_Transmit(_hi2c, ADS1115_I2C_ADDR << 1, buffer, 3, HAL_MAX_DELAY);

  // 변환 완료 대기
  HAL_Delay(10);

  // 포인터 레지스터를 Conversion 레지스터로 설정
  buffer[0] = ADS1115_REG_CONVERSION;
  HAL_I2C_Master_Transmit(_hi2c, ADS1115_I2C_ADDR << 1, buffer, 1, HAL_MAX_DELAY);

  // 변환 결과 읽기
  HAL_I2C_Master_Receive(_hi2c, ADS1115_I2C_ADDR << 1, buffer, 2, HAL_MAX_DELAY);

  // MSB와 LSB 결합
  adc_value = ((int16_t)buffer[0] << 8) | buffer[1];

  return adc_value;
}

/**
  * @brief 여러 샘플을 읽어서 평균값 반환
  * @retval 평균 ADC 값
  */
int16_t ADS1115_ReadAveragedADC(void) {
  int32_t sum = 0;

//  for(int i = 0; i < 5; i++) {
//    sum += ADS1115_ReadADC();
//    HAL_Delay(10);  // 샘플 간 약간의 지연
//  }
  sum = ADS1115_ReadADC();
  HAL_Delay(10);

  //return sum / SAMPLES_COUNT;
  return sum;
}

/**
  * @brief 영점 캘리브레이션 수행
  * @retval None
  */
void Calibrate_Zero(void) {
  printf("Starting zero calibration...\r\n");
  printf("Please remove all weight from the load cell\r\n");
  HAL_Delay(3000);  // 대기 시간

  int32_t sum = 0;
  // 여러 샘플을 읽어서 평균 계산
  for(int i = 0; i < SAMPLES_COUNT; i++) {
    sum += ADS1115_ReadADC();
    HAL_Delay(100);
  }

  adc_offset = sum / SAMPLES_COUNT;
  printf("Zero calibration complete. Offset value: %d\r\n", adc_offset);
}

/**
  * @brief 알려진 무게로 스케일 팩터 캘리브레이션
  * @param known_weight 실제 무게 (g 또는 kg 단위)
  * @retval None
  */
void Calibrate_Scale(float known_weight) {
  printf("Starting scale calibration...\r\n");
  printf("Please place %.2f g reference weight on the load cell\r\n", known_weight);
  HAL_Delay(5000);  // 충분한 대기 시간

  int32_t sum = 0;
  // 여러 샘플을 읽어서 평균 계산
  for(int i = 0; i < SAMPLES_COUNT; i++) {
    sum += ADS1115_ReadADC();
    HAL_Delay(100);
  }

  int16_t calibration_value = sum / SAMPLES_COUNT;

  // 스케일 팩터 계산: 실제무게 / (ADC값 - 오프셋)
  scale_factor = known_weight / (float)(calibration_value - adc_offset);

  printf("Scale calibration complete. Scale factor: %.6f\r\n", scale_factor);
  is_calibrated = true;
}

/* Getter functions for calibration data */
int16_t ADS1115_GetOffset(void) {
  return adc_offset;
}

float ADS1115_GetScaleFactor(void) {
  return scale_factor;
}

bool ADS1115_IsCalibrated(void) {
  return is_calibrated;
}
