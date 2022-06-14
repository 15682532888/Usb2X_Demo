/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "main.h"
#include "Can.h"
#include "Lin.h"
#include "KLine.h"
#include "usart.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CMD_MAX                      (uint8_t)0x0f
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
static void cmd_TransmitStdFrame2Can(uint8_t* data, uint8_t len);

static void cmd_SetCanBps(uint8_t* data, uint8_t len);

static void cmd_SetCanRxFilter01(uint8_t* data, uint8_t len);

static void cmd_SetCanRxFilter10(uint8_t* data, uint8_t len);

static void cmd_StartCan(uint8_t* data, uint8_t len);

static void cmd_StopCan(uint8_t* data, uint8_t len);

static void cmd_Test(uint8_t* data, uint8_t len);

static void cmd_KLine25msInitAndStart(uint8_t* data, uint8_t len);

static void cmd_KLineSendData(uint8_t* data, uint8_t len);

static void KLineSetCs(uint8_t* data, uint8_t len);

static void cmd_KLineSetBps(uint8_t* data, uint8_t len);

static void cmd_LinSendData(uint8_t* data, uint8_t len);
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */
/*Command type define for upper PC*/
typedef void(*cmdFuncType)(uint8_t* data, uint8_t len);

/*All support command from upper PC*/
static const cmdFuncType cmdFuncTable[CMD_MAX] =
{
	cmd_TransmitStdFrame2Can,
	cmd_SetCanBps,
	cmd_SetCanRxFilter01,
	cmd_SetCanRxFilter10,
	cmd_StartCan,
	cmd_StopCan,
	cmd_Test,
	RT_NULL,
	cmd_KLine25msInitAndStart,
	cmd_KLineSendData,
	cmd_KLineSetBps,
	cmd_LinSendData,
	cmd_LinStart,
	RT_NULL,
	RT_NULL
};
/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
static uint8_t usbData[1024];
/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
typedef struct
{
	uint16_t dataLength;
	uint16_t dataIndex;
	uint8_t usbData[4 * 1024];
	uint8_t frameCount;
} usbDataBufferType;

usbDataBufferType usbDataBuffer;

static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);

  if ((3 < *Len) && (0xa5 == Buf[0]) && (0x5a == Buf[1]))
  {
	  const uint8_t pci = (0xf0 & Buf[2]);
	  const uint8_t pciOther = (0x0f & Buf[2]);

	  if (0x00 == pci)
	  {
		  /*signal frame*/
		  usbDataBuffer.dataLength = (16 * pciOther + Buf[3]);
		  usbDataBuffer.dataIndex = 0u;

		  /*copy the valid data start cmd byte*/
		  while (usbDataBuffer.dataIndex < usbDataBuffer.dataLength)
		  {
			  usbDataBuffer.usbData[usbDataBuffer.dataIndex] = Buf[4 + usbDataBuffer.dataIndex];
			  usbDataBuffer.dataIndex++;
		  }
	  }
	  else if (0x10 == pci)
	  {
		  /*first frame*/
		  usbDataBuffer.dataLength = (16 * pciOther + Buf[3]);
		  usbDataBuffer.dataIndex = 0u;
		  usbDataBuffer.frameCount = 1u;

		  /*copy the whole data start cmd byte*/
		  while (usbDataBuffer.dataIndex < (64 - 4))
		  {
			  usbDataBuffer.usbData[usbDataBuffer.dataIndex] = Buf[4 + usbDataBuffer.dataIndex];
			  usbDataBuffer.dataIndex++;
		  }
	  }
	  else if (0x20 == pci)
	  {
		  if (pciOther == usbDataBuffer.frameCount)
		  {
			  /*copy the flow frame data*/
			  for (uint8_t i = 0; ((i < (64 - 3)) & (usbDataBuffer.dataIndex < usbDataBuffer.dataLength)); i++)
			  {
				  usbDataBuffer.usbData[usbDataBuffer.dataIndex] = Buf[3 + i];
				  usbDataBuffer.dataIndex++;
			  }
			  usbDataBuffer.frameCount++;

			  if (0x0f < usbDataBuffer.frameCount)
			  {
				  usbDataBuffer.frameCount = 0;
			  }
		  }
		  else
		  {
			  /*frame count error*/
		  }
	  }
	  else
	  {
		  /*pci error*/
	  }

	  if (usbDataBuffer.dataIndex == usbDataBuffer.dataLength)
	  {
			const uint8_t funcIndex = usbDataBuffer.usbData[0];
			if (funcIndex < CMD_MAX)
			{
				cmdFuncTable[funcIndex](&(usbDataBuffer.usbData[1]), usbDataBuffer.dataLength - 1);
			}
	  }
  }

  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);

  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
static void cmd_TransmitStdFrame2Can(uint8_t* data, uint8_t len)
{
	CAN_TxHeaderTypeDef pHeader;
	uint32_t pTxMailbox;

	pHeader.DLC = len - 2;
	pHeader.StdId = ((data[0] << 8) | data[1]);
	pHeader.IDE = CAN_ID_STD;
	pHeader.RTR = CAN_RTR_DATA;

	if ( HAL_OK != HAL_CAN_AddTxMessage(&hcan2, &pHeader, &(data[2]), &pTxMailbox))
	{
	  Error_Handler();
	}

	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
}

static void cmd_SetCanBps(uint8_t* data, uint8_t len)
{
	const uint16_t bps = ((data[0] << 8) | data[1]);

	HAL_CAN_DeInit(&hcan2);

	switch(bps)
	{
		case 125:
			hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
			hcan2.Init.TimeSeg1 = CAN_BS1_6TQ;
			hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
			hcan2.Init.Prescaler = 36;
		break;
		case 250:
			hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
			hcan2.Init.TimeSeg1 = CAN_BS1_3TQ;
			hcan2.Init.TimeSeg2 = CAN_BS2_5TQ;
			hcan2.Init.Prescaler = 16;
		break;
		case 500:
			hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
			hcan2.Init.TimeSeg1 = CAN_BS1_6TQ;
			hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
			hcan2.Init.Prescaler = 9;
		break;
		case 1000:
			hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
			hcan2.Init.TimeSeg1 = CAN_BS1_2TQ;
			hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
			hcan2.Init.Prescaler = 9;
		break;
		default:
			hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
			hcan2.Init.TimeSeg1 = CAN_BS1_6TQ;
			hcan2.Init.TimeSeg2 = CAN_BS2_1TQ;
			hcan2.Init.Prescaler = 9;
			break;
	}

	if (HAL_CAN_Init(&hcan2) != HAL_OK)
	{
		Error_Handler();
	}
}

static void cmd_SetCanRxFilter01(uint8_t* data, uint8_t len)
{
	uint16_t id1 = ((data[0] << 8) | data[1]);
	uint16_t id2 = ((data[2] << 8) | data[3]);
	uint16_t id3 = ((data[4] << 8) | data[5]);
	uint16_t id4 = ((data[6] << 8) | data[7]);

	CAN_FilterTypeDef CAN2_FilerConf;

	CAN2_FilerConf.FilterIdHigh = (id1 << 5);
	CAN2_FilerConf.FilterIdLow = (id2 << 5);
	CAN2_FilerConf.FilterMaskIdHigh = (id3 << 5);
	CAN2_FilerConf.FilterMaskIdLow = (id4 << 5);
	CAN2_FilerConf.FilterFIFOAssignment = CAN_FILTER_FIFO1;
	CAN2_FilerConf.FilterBank = 14;
	CAN2_FilerConf.FilterMode = CAN_FILTERMODE_IDLIST;
	CAN2_FilerConf.FilterScale = CAN_FILTERSCALE_16BIT;
	CAN2_FilerConf.FilterActivation = ENABLE;
	CAN2_FilerConf.SlaveStartFilterBank = 14;

	if (HAL_CAN_ConfigFilter(&hcan2, &CAN2_FilerConf) != HAL_OK)
	{
		Error_Handler();
	}
}

static void cmd_SetCanRxFilter10(uint8_t* data, uint8_t len)
{
	uint16_t id1 = ((data[0] << 8) | data[1]);
	uint16_t id2 = ((data[2] << 8) | data[3]);
	uint16_t mask1 = ((data[4] << 8) | data[5]);
	uint16_t mask2 = ((data[6] << 8) | data[7]);

	CAN_FilterTypeDef CAN2_FilerConf;

	CAN2_FilerConf.FilterIdHigh = id1;
	CAN2_FilerConf.FilterIdLow = (id2 << 5);
	CAN2_FilerConf.FilterMaskIdHigh = mask1;
	CAN2_FilerConf.FilterMaskIdLow = (mask2 << 5);
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
}

static void cmd_StartCan(uint8_t* data, uint8_t len)
{
	if (HAL_CAN_STATE_READY == hcan2.State)
	{
		if (HAL_CAN_Start(&hcan2) != HAL_OK)
		{
			Error_Handler();
		}

		if (HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING |CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

static void cmd_StopCan(uint8_t* data, uint8_t len)
{
	if (HAL_CAN_STATE_LISTENING == hcan2.State)
	{
		(void)HAL_CAN_Stop(&hcan2);
	}
}

cmd_ResponseType cmd_Response;

static void cmd_Test(uint8_t* data, uint8_t len)
{
	cmd_Response.needResp = RT_TRUE;
	cmd_Response.data[0] = 0xff;

	for (uint8_t i = 1; i < 64; i++)
	{
		cmd_Response.data[i] = 0;
	}
}

void Delay(uint16_t nCount)
{
	uint16_t i=0;
	while(nCount--)
	{
	   i=7920;
	   while(i--);
	}
}

static void cmd_KLine25msInitAndStart(uint8_t* data, uint8_t len)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	uint8_t bps[2] = {0x28, 0xa0};

	if (USART3 == huart3.Instance)
	{
		(void)HAL_UART_DeInit(&huart3);
	}

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
	Delay(25);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
	Delay(24);

	cmd_KLineSetBps(bps, 2);
	cmd_KLineSendData(data, len);
}

static void cmd_KLineSendData(uint8_t* data, uint8_t len)
{
	KLineSetCs(data, len);
	(void)HAL_UART_Transmit(&huart3, data, len + 1, 10);
	(void)HAL_UARTEx_ReceiveToIdle_IT(&huart3, usart_RxDataBuffer, KLINE_RESPONSE_LENGTH_MAX);
}

static void KLineSetCs(uint8_t* data, uint8_t len)
{
	uint8_t cs = 0u;
	uint8_t index = 0u;

	while (index < len)
	{
		cs += data[index];
		index++;
	}

	/*reuse the last plus one buffer, no error, it is 4K buffer ,enough for one KLine frame*/
	data[len] = cs;
}

static void cmd_KLineSetBps(uint8_t* data, uint8_t len)
{
	uint32_t bps = ((data[0] << 8) + data[1]);

	if (USART3 == huart3.Instance)
	{
		(void)HAL_UART_DeInit(&huart3);
	}

	huart3.Instance = USART3;
	huart3.Init.BaudRate = bps;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart3) != HAL_OK)
	{
		Error_Handler();
	}
}

Lin_TpSendDataType tpSendData;
static void cmd_LinSendData(uint8_t* data, uint8_t len)
{
	tpSendData.Lin_TpReceiveEndFunc = RT_NULL;
	tpSendData.Lin_TpSendCbkFunc = RT_NULL;
	tpSendData.data = data;
	tpSendData.dataLen = len;
	tpSendData.nad = 0x04;
	tpSendData.needResp = 1;
	Lin_TpSendData(&tpSendData);
}

void Usb_MainFunction(void)
{
	/*first handle all the command*/
	if (RT_TRUE == cmd_Response.needResp)
	{
		if (USBD_OK == CDC_Transmit_FS(cmd_Response.data, 64))
		{
			cmd_Response.needResp = RT_FALSE;
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
		}
	}
	else
	{
		const uint8_t nextIndex = Can_RxDataBuffer.nextIndex;
		const uint8_t canFrameCount = Can_RxDataBuffer.count;
		const uint8_t index = ((CAN_DATA_BUFFER_LENGTH - canFrameCount + nextIndex) % CAN_DATA_BUFFER_LENGTH);
		/*four can frame can put in one 64bytes usb frame*/
		const uint8_t usbFrameFullCount = (canFrameCount / 4);

		/*send as more in one large usb frame*/
		if (0u < usbFrameFullCount)
		{
			uint8_t j;
			/*most 16 * 64 = 1024bytes in one usb transfer*/
			for (j = 0; ((j < usbFrameFullCount) && (j < 16)); j++)
			{
				usbData[0 + j * 64] = 4;
				for (uint8_t i = 0; i < 14; i++)
				{
					usbData[8 + i + j * 64] = Can_RxDataBuffer.data[index + j * 4][i];
					usbData[22 + i + j * 64] = Can_RxDataBuffer.data[index + 1 + j * 4][i];
					usbData[36 + i + j * 64] = Can_RxDataBuffer.data[index + 2 + j * 4][i];
					usbData[50 + i + j * 64] = Can_RxDataBuffer.data[index + 3 + j * 4][i];
				}
			}

			if (USBD_OK == CDC_Transmit_FS(usbData, j * 64))
			{
				Can_RxDataBuffer.count -= (4u * j);
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
			}
			else
			{
				Error_Handler();
			}
		}
		else if (0u < canFrameCount)
		{
			usbData[0] = canFrameCount;
			for (uint8_t frameIndex = 0; frameIndex < canFrameCount; frameIndex++)
			{
				for (uint8_t i = 0; i < 14; i++)
				{
					usbData[8 + 14 * frameIndex + i] = Can_RxDataBuffer.data[index + frameIndex][i];
				}
			}

			for (uint8_t i = 0; i < 64 - 8 - canFrameCount * 14; i++)
			{
				usbData[8 + 14 * canFrameCount + i] = 0;
			}

			if (USBD_OK == CDC_Transmit_FS(usbData, 64))
			{
				Can_RxDataBuffer.count -= canFrameCount;
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
			}
			else
			{
				Error_Handler();
			}
		}
		else
		{
			/*do nothing*/
		}
	}
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
