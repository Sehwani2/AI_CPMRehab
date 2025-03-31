/*
 * HX711.c
 *
 *  Created on: Mar 29, 2025
 *      Author: msi
 */
#include "hx711.h"
#include <stdio.h>

static int32_t offset = 0;
static float scale_factor = 67000.0f;  // 65.58625 * 1000 (더 정확한 값으로 조정)

void HX711_Init(void) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_Delay(100);

    // 초기 25번의 클럭을 보내서 채널A, 게인 128 설정
    for(int i = 0; i < 25; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    }

    HX711_SetScale(scale_factor);

    // 초기화 시 자동 영점 조정 및 오프셋 출력
    HAL_Delay(1000);  // 안정화 대기
    int32_t sum = 0;
    for(int i = 0; i < 30; i++) {
        sum += HX711_ReadValue();
        HAL_Delay(50);
    }
    offset = sum / 30;
    printf("HX711 Init - Offset: %ld\r\n", offset);  // 오프셋 값 출력
}

void HX711_SetScale(float factor) {
    scale_factor = factor;
}

int32_t HX711_ReadValue(void) {
    int32_t value = 0;

    while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET);

    for(int i = 0; i < 24; i++) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
        value = value << 1;

        if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET) {
            value++;
        }

        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    }

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

    value = value ^ 0x800000;

    return value;
}

float HX711_GetUnits(uint8_t times) {
    int32_t sum = 0;

    for(uint8_t i = 0; i < times; i++) {
        sum += HX711_ReadValue();
        HAL_Delay(10);
    }

    int32_t value = sum / times;
    return (float)(value - offset) / scale_factor * 1000.0f;
}

void HX711_Tare(void) {
    int32_t sum = 0;

    for(int i = 0; i < 20; i++) {  // 샘플 수 증가
        sum += HX711_ReadValue();
        HAL_Delay(50);  // 지연 시간 증가
    }

    offset = sum / 20;
    printf("Tare offset: %ld\r\n", offset);
}

int HX711_GetWeight(void) {
    static int filtered_weight = 0;
    int32_t raw_value = HX711_ReadValue();

    int current_weight = (int)((raw_value - offset) / scale_factor * 1000.0f);
    // 85:15 비율의 필터링을 정수 연산으로 변경 (85% : 15%)
    filtered_weight = (filtered_weight * 85 + current_weight * 15) / 100;
    return filtered_weight;
}
