/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan2;

/* USER CODE BEGIN Private defines */
typedef struct
{
	uint8_t needResp;
	uint8_t respLen;
	uint8_t data[64];
} cmd_ResponseType;

extern cmd_ResponseType cmd_Response;

#define CAN_DATA_BUFFER_LENGTH         100
typedef uint8_t canBuffer[2 + 8 + 4];
typedef struct
{
	uint8_t count;
	uint8_t nextIndex;
	canBuffer data[CAN_DATA_BUFFER_LENGTH];
} Can_RxDataBufferType;

extern Can_RxDataBufferType Can_RxDataBuffer;
/* USER CODE END Private defines */

void MX_CAN2_Init(void);

/* USER CODE BEGIN Prototypes */
void Can_Init(void);
void Can_TxConfirmation(void);
void Can_RxIndication(CAN_HandleTypeDef *hcan);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

