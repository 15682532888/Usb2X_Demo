/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
Can_RxDataBufferType Can_RxDataBuffer;

void Can_Init(void)
{
	Can_RxDataBuffer.count = 0;
	Can_RxDataBuffer.nextIndex = 0;
	cmd_Response.needResp = RT_FALSE;
}
/* USER CODE END 0 */

CAN_HandleTypeDef hcan2;

/* CAN2 init function */
void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */
	CAN_FilterTypeDef CAN2_FilerConf;
  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 9;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */
  CAN2_FilerConf.FilterIdHigh = 0x0000;
  CAN2_FilerConf.FilterIdLow = 0x0000;
  CAN2_FilerConf.FilterMaskIdHigh = 0x0000;
  CAN2_FilerConf.FilterMaskIdLow = 0x0000;
  CAN2_FilerConf.FilterFIFOAssignment = CAN_FILTER_FIFO1;
  CAN2_FilerConf.FilterBank = 14;
  CAN2_FilerConf.FilterMode = CAN_FILTERMODE_IDMASK;
  CAN2_FilerConf.FilterScale = CAN_FILTERSCALE_16BIT;
  CAN2_FilerConf.FilterActivation = ENABLE;
  CAN2_FilerConf.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(&hcan2, &CAN2_FilerConf) != HAL_OK)
  {
	  Error_Handler();
  }
  /* USER CODE END CAN2_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspInit 0 */

  /* USER CODE END CAN2_MspInit 0 */
    /* CAN2 clock enable */
    __HAL_RCC_CAN2_CLK_ENABLE();
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN2 interrupt Init */
    HAL_NVIC_SetPriority(CAN2_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspInit 1 */

  /* USER CODE END CAN2_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN2)
  {
  /* USER CODE BEGIN CAN2_MspDeInit 0 */

  /* USER CODE END CAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN2_CLK_DISABLE();
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN2 GPIO Configuration
    PB12     ------> CAN2_RX
    PB13     ------> CAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);

    /* CAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN2_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
  /* USER CODE BEGIN CAN2_MspDeInit 1 */

  /* USER CODE END CAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void Can_TxConfirmation(void)
{

}

void Can_RxIndication(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxMsg;
	uint8_t nextIndex = Can_RxDataBuffer.nextIndex;

	if (HAL_OK == HAL_CAN_GetRxMessage(&hcan2, CAN_RX_FIFO1, &RxMsg, &(Can_RxDataBuffer.data[nextIndex][2])))
	{
		uint32_t timeNow = Tm_ResetTimer100us32bit();
		Can_RxDataBuffer.data[nextIndex][0] = (0xff & (RxMsg.StdId >> 8));
		Can_RxDataBuffer.data[nextIndex][1] = (0xff & (RxMsg.StdId >> 0));
		Can_RxDataBuffer.data[nextIndex][10] = (0xff & (timeNow >> 24));
		Can_RxDataBuffer.data[nextIndex][11] = (0xff & (timeNow >> 16));
		Can_RxDataBuffer.data[nextIndex][12] = (0xff & (timeNow >> 8));
		Can_RxDataBuffer.data[nextIndex][13] = (0xff & (timeNow >> 0));

		Can_RxDataBuffer.count++;
		nextIndex = ((nextIndex + 1) % CAN_DATA_BUFFER_LENGTH);
		Can_RxDataBuffer.nextIndex = nextIndex;

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
	}
}
/* USER CODE END 1 */
