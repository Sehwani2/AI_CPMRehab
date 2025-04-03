/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : ads1115.h
  * @brief          : Header for ads1115.c file.
  *                   This file contains ADS1115 ADC definitions and functions.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __ADS1115_H
#define __ADS1115_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ADS1115 Definitions */
// ADS1115 I2C address (ADDR connected to GND = 0x48)
#define ADS1115_I2C_ADDR        0x48

// ADS1115 registers
#define ADS1115_REG_CONVERSION  0x00
#define ADS1115_REG_CONFIG      0x01
#define ADS1115_REG_LO_THRESH   0x02
#define ADS1115_REG_HI_THRESH   0x03

// Config register bits
// Bits 15:8
#define ADS1115_OS_SINGLE       0x8000  // Start a single conversion
#define ADS1115_MUX_DIFF_0_1    0x0000  // Differential P=AIN0, N=AIN1
#define ADS1115_MUX_SINGLE_0    0x4000  // Single-ended AIN0 vs GND
#define ADS1115_PGA_0_256V      0x0500  // ±0.256V range = Gain 16
#define ADS1115_PGA_0_512V      0x0400  // ±0.512V range = Gain 8
#define ADS1115_PGA_1_024V      0x0300  // ±1.024V range = Gain 4
#define ADS1115_PGA_2_048V      0x0200  // ±2.048V range = Gain 2
#define ADS1115_PGA_4_096V      0x0100  // ±4.096V range = Gain 1
#define ADS1115_PGA_6_144V      0x0000  // ±6.144V range = Gain 2/3
#define ADS1115_MODE_SINGLE     0x0100  // Single-shot mode
#define ADS1115_MODE_CONT       0x0000  // Continuous mode

// Bits 7:0
#define ADS1115_DR_8SPS         0x0000  // 8 samples per second
#define ADS1115_DR_16SPS        0x0020  // 16 samples per second
#define ADS1115_DR_32SPS        0x0040  // 32 samples per second
#define ADS1115_DR_64SPS        0x0060  // 64 samples per second
#define ADS1115_DR_128SPS       0x0080  // 128 samples per second (default)
#define ADS1115_DR_250SPS       0x00A0  // 250 samples per second
#define ADS1115_DR_475SPS       0x00C0  // 475 samples per second
#define ADS1115_DR_860SPS       0x00E0  // 860 samples per second
#define ADS1115_COMP_MODE_TRAD  0x0000  // Traditional comparator
#define ADS1115_COMP_POL_ACTVLOW 0x0000 // Active low
#define ADS1115_COMP_LAT        0x0004  // Latching comparator
#define ADS1115_COMP_QUE_DIS    0x0003  // Disable comparator and set ALERT pin to high-impedance

#define SAMPLES_COUNT  20       // 평균화를 위한 샘플 수

/* Function Prototypes */
void ADS1115_SetI2CHandle(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef ADS1115_Init(void);
void Calibrate_Zero(void);

int16_t ADS1115_ReadADC(void);
int16_t ADS1115_ReadAveragedADC(void);
void Calibrate_Scale(float known_weight);

/* Get functions for calibration data */
int16_t ADS1115_GetOffset(void);
float ADS1115_GetScaleFactor(void);
bool ADS1115_IsCalibrated(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADS1115_H */
