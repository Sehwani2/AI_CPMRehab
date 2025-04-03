/*
 * HX711.h
 *
 *  Created on: Mar 31, 2025
 *      Author: user
 */

#ifndef INC_HX711_H_
#define INC_HX711_H_

#include "main.h"

void HX711_Init(void);
int32_t HX711_ReadValue(void);
void HX711_Tare(void);
int HX711_GetWeight(void);
void HX711_SetScale(float scale_factor);
float HX711_GetUnits(uint8_t times);


#endif /* INC_HX711_H_ */
