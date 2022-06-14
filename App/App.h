/*
 * App.h
 *
 *  Created on: May 2, 2022
 *      Author: cheng
 */

#ifndef APP_H
#define APP_H

#include "main.h"

void Tm_Init();

void Tm_LowTimePeriodElapsed();

uint32_t Tm_ResetTimer100us32bit();

uint32_t Tm_GetTimeSpan100us32bit(const uint32_t timer);

#endif /* APP_H */
