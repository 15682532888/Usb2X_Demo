/*
 * App.c
 *
 *  Created on: 2022年5月14日
 *      Author: cheng
 */

#include "main.h"
#include "tim.h"

static uint16_t globalTimeHigh;

void Tm_Init()
{
	globalTimeHigh = 0;
}

uint32_t Tm_GetTimeSpan100us32bit(const uint32_t timer)
{
	return (uint32_t)(((globalTimeHigh << 16) | (uint16_t)__HAL_TIM_GetCounter(&htim1)) - timer);
}

uint32_t Tm_ResetTimer100us32bit()
{
	return (uint32_t)((globalTimeHigh << 16) | (uint16_t)__HAL_TIM_GetCounter(&htim1));
}

void Tm_LowTimePeriodElapsed()
{
	globalTimeHigh++;
}
