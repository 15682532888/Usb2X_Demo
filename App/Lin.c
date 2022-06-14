/*
 * Lin.c
 *
 *  Created on: 2022年5月14日
 *      Author: cheng
 */

#include "Lin.h"
#include "Can.h"

Lin_ProessStatusType Lin_ProessStatus;

static void Lin_HandleTxCFFrame(Lin_ProessStatusType* proessStatus);
static void Lin_TpReceiveFFOrSFFrame(uint8_t *data);
static void Lin_TpReceiveCFFrame();
static void Lin_TpSendFrame(Lin_FrameInfoType *frameInfo, Lin_ProessStatusType* proessStatus);
static void Lin_ResetProcess(Lin_ProessStatusType* proessStatus);
static uint8_t Lin_CalcPid(uint8_t id);
static uint8_t Lin_CheckSum(uint8_t enhanced, uint8_t pid, uint8_t *data, uint8_t dataLen);
static void LIN_SendHead(Lin_FrameInfoType *frameInfo);
static void Lin_HandleTxFFOrFFFrame(Lin_ProessStatusType *proessStatus);

void Lin_Init(void)
{
	Lin_ResetProcess(&Lin_ProessStatus);
}

void cmd_LinStart(uint8_t* data, uint8_t len)
{
	if (USART3 == huart3.Instance)
	{
		(void)HAL_UART_DeInit(&huart3);
	}

	MX_USART3_UART_Init();
}

void cmd_LinStop(uint8_t* data, uint8_t len)
{
	if (USART3 == huart3.Instance)
	{
		(void)HAL_UART_DeInit(&huart3);
	}
}

void Lin_TpSendData(const Lin_TpSendDataType* tpSendData)
{
	Lin_ResetProcess(&Lin_ProessStatus);
	Lin_ProessStatus.data = tpSendData->data;
	Lin_ProessStatus.dataLength = tpSendData->dataLen;
	Lin_ProessStatus.mainStatus = LIN_STATUS_PROESSING;
	Lin_ProessStatus.subStatus = LIN_SUB_STATUS_SENDING;
	Lin_ProessStatus.needResp = tpSendData->needResp;
	Lin_ProessStatus.nad = tpSendData->nad;
	Lin_ProessStatus.tpSendData = tpSendData;
	Lin_HandleTxFFOrFFFrame(&Lin_ProessStatus);
}

void Lin_MainFunction(void)
{
	if (LIN_STATUS_IDLE != Lin_ProessStatus.mainStatus)
	{
		switch (Lin_ProessStatus.tpTimer)
		{
			case LIN_TP_N_AS:
			{
				if (10000u < Tm_GetTimeSpan100us32bit(Lin_ProessStatus.time))
				{
					if (RT_NULL != Lin_ProessStatus.tpSendData->Lin_TpSendCbkFunc)
					{
						Lin_ProessStatus.tpSendData->Lin_TpSendCbkFunc(1);
					}
					Lin_ResetProcess(&Lin_ProessStatus);
				}
			};
			break;
			case LIN_TP_N_CS:
			{
				if (9000u > Tm_GetTimeSpan100us32bit(Lin_ProessStatus.time))
				{
					Lin_HandleTxCFFrame(&Lin_ProessStatus);
				}
				else
				{
					if (RT_NULL != Lin_ProessStatus.tpSendData->Lin_TpSendCbkFunc)
					{
						Lin_ProessStatus.tpSendData->Lin_TpSendCbkFunc(1);
					}

					Lin_ResetProcess(&Lin_ProessStatus);
				}

			};
			break;
			case LIN_TP_N_CR:
			{
				if (10000u > Tm_GetTimeSpan100us32bit(Lin_ProessStatus.time))
				{
					if (LIN_SUB_STATUS_PRE_RECEIVE == Lin_ProessStatus.subStatus)
					{
						Lin_TpReceiveFFOrSFFrame(usart_RxDataBuffer);
						Lin_ProessStatus.subStatus = LIN_SUB_STATUS_RECEIVING;
					}
					else if (LIN_SUB_STATUS_RECEIVING == Lin_ProessStatus.subStatus)
					{
						Lin_TpReceiveCFFrame();
					}
					else
					{
						/*do nothing*/
					}
				}
				else
				{
					if (RT_NULL != Lin_ProessStatus.tpSendData->Lin_TpReceiveEndFunc)
					{
						Lin_ProessStatus.tpSendData->Lin_TpReceiveEndFunc(1, Lin_ProessStatus.dataLength);
					}

					Lin_ResetProcess(&Lin_ProessStatus);
				}
			};
			break;
			default:
				/*do nothing*/
				break;
		}
	}
}

static void Lin_ResetProcess(Lin_ProessStatusType* proessStatus)
{
	proessStatus->mainStatus = LIN_STATUS_IDLE;
	proessStatus->subStatus = LIN_SUB_STATUS_IDLE;
}

static uint8_t Lin_CheckSum(uint8_t enhanced, uint8_t pid, uint8_t *data, uint8_t dataLen)
{
    uint8_t ckm;
    uint16_t chm1 = pid;

    if (0 == enhanced)
    {
    	chm1 = 0;
    }

    for(uint8_t i = 0; i < dataLen; i++)
    {
        chm1 += *(data++);
    }
    ckm = (chm1 >> 8);
    ckm += (chm1 % 256u);
    ckm = ~ckm;

    return ckm;
}

static uint8_t Lin_CalcPid(uint8_t id)
{
  uint8_t pid, p0,p1;

  pid = id;
  p0 = (BIT(pid, 0) ^ BIT(pid, 1) ^ BIT(pid, 2) ^ BIT(pid, 4)) << 6;
  p1 = (!(BIT(pid, 1) ^ BIT(pid, 3) ^ BIT(pid, 4) ^ BIT(pid, 5))) << 7;
  pid |= (p0 | p1);

  return pid;
}

static void LIN_SendHead(Lin_FrameInfoType *frameInfo)
{
	uint8_t snycData = 0x55;
	uint8_t pid = Lin_CalcPid(frameInfo->id);

	(void)HAL_LIN_SendBreak(&huart3);
	(void)HAL_UART_Transmit(&huart3, &snycData, 1, LIN_TIMEOUT_10MS);
	(void)HAL_UART_Transmit(&huart3, &pid, 1, LIN_TIMEOUT_10MS);
}

static void Lin_HandleTxFFOrFFFrame(Lin_ProessStatusType *proessStatus)
{
	Lin_FrameInfoType frameInfo;

	frameInfo.id = 0x3C;
	frameInfo.data[0] = proessStatus->nad;
	frameInfo.length = 8;
	proessStatus->dataIndex = 0u;

	if (6 < proessStatus->dataLength)
	{
		/*first frame*/
		frameInfo.data[1] = (0x10 | ( proessStatus->dataLength >> 8));
		frameInfo.data[2] = ( proessStatus->dataLength % 256);
		for (uint8_t i = 0; i < 5; i++)
		{
			frameInfo.data[3 + i] = proessStatus->data[proessStatus->dataIndex];
			proessStatus->dataIndex++;
		}

		proessStatus->frameCounter = 0x01;
	}
	else
	{
		/*signal frame*/
		for (uint8_t i = 0; i < 6; i++)
		{
			if (i < proessStatus->dataLength)
			{
				frameInfo.data[2 + i] = proessStatus->data[proessStatus->dataIndex];
				proessStatus->dataIndex++;
			}
			else
			{
				frameInfo.data[2 + i] = 0xff;
			}
		}

		frameInfo.data[1] = proessStatus->dataIndex;
	}

	Lin_TpSendFrame(&frameInfo, proessStatus);
}

static void Lin_HandleTxCFFrame(Lin_ProessStatusType* proessStatus)
{
	Lin_FrameInfoType frameInfo;
	frameInfo.length = 2;

	frameInfo.id = 0x3C;
	frameInfo.data[0] = proessStatus->nad;
	frameInfo.length = 8;
	frameInfo.data[1] = (0x20 | (0x0f & proessStatus->frameCounter++));

	for (uint8_t i = 0; i < 6; i++)
	{
		if (proessStatus->dataIndex < proessStatus->dataLength)
		{
			frameInfo.data[2 + i] = proessStatus->data[proessStatus->dataIndex];
			proessStatus->dataIndex++;
		}
		else
		{
			frameInfo.data[2 + i] = 0xff;
		}
	}

	if (0x0f < proessStatus->frameCounter)
	{
		proessStatus->frameCounter = 0;
	}

	Lin_TpSendFrame(&frameInfo, proessStatus);
}

static void Lin_TpSendFrame(Lin_FrameInfoType *frameInfo, Lin_ProessStatusType* proessStatus)
{
	uint8_t ckm = Lin_CheckSum(0, Lin_CalcPid(frameInfo->id), frameInfo->data, frameInfo->length);

	proessStatus->tpTimer = LIN_TP_N_AS;
	proessStatus->time = Tm_ResetTimer100us32bit();

    LIN_SendHead(frameInfo);
    (void)HAL_UART_Transmit(&huart3, frameInfo->data, frameInfo->length, LIN_TIMEOUT_10MS);
	(void)HAL_UART_Transmit_IT(&huart3, &ckm, 1);
}

static void Lin_TpReceiveFFOrSFFrame(uint8_t *data)
{
	Lin_FrameInfoType frameInfo;

	frameInfo.id = 0x3D;
	Lin_ProessStatus.data = data;
	Lin_ProessStatus.dataIndex = 0;
	LIN_SendHead(&frameInfo);
	(void)HAL_UARTEx_ReceiveToIdle_IT(&huart3, Lin_ProessStatus.dataBuffer, 8);
}

static void Lin_TpReceiveCFFrame()
{
    Lin_FrameInfoType frameInfo;

    frameInfo.id = 0x3D;
    LIN_SendHead(&frameInfo);
	(void)HAL_UARTEx_ReceiveToIdle_IT(&huart3, Lin_ProessStatus.dataBuffer, 8);
}

void Lin_TpTxConfirmation(Lin_ProessStatusType* processStatusPtr)
{
	if (LIN_SUB_STATUS_SENDING == processStatusPtr->subStatus)
	{
		if (LIN_TP_N_AS == processStatusPtr->tpTimer)
		{
			if (processStatusPtr->dataLength != processStatusPtr->dataIndex)
			{
				processStatusPtr->tpTimer = LIN_TP_N_CS;
				processStatusPtr->time = Tm_ResetTimer100us32bit();
			}
			else
			{
				if (RT_NULL != processStatusPtr->tpSendData->Lin_TpSendCbkFunc)
				{
					processStatusPtr->tpSendData->Lin_TpSendCbkFunc(0);
				}

				/*CF or SF transmit end, if need receive*/
				if (0 != processStatusPtr->needResp)
				{
					processStatusPtr->subStatus = LIN_SUB_STATUS_PRE_RECEIVE;
					processStatusPtr->tpTimer = LIN_TP_N_CR;
					processStatusPtr->time = Tm_ResetTimer100us32bit();
				}
				else
				{
					Lin_ResetProcess(processStatusPtr);
				}
			}
		}
	}
}

void Lin_TpRxIndication(Lin_ProessStatusType* processStatusPtr)
{
	const uint8_t pci = (0xF0 & processStatusPtr->dataBuffer[1]);
	const uint8_t firstByteLow = (0x0F & processStatusPtr->dataBuffer[1]);

	if (0x00 == pci)
	{
		/*response is nrc78, handle it*/
		if ((0x78 == processStatusPtr->dataBuffer[4])
			&& (0x7f == processStatusPtr->dataBuffer[2])
			&& (0x03 == firstByteLow))
		{
			/*need send Lin header for follow response*/
			processStatusPtr->tpTimer = LIN_TP_N_CR;
			processStatusPtr->time = Tm_ResetTimer100us32bit();
		}
		else
		{
			/*response is sf*/
			Lin_ResetProcess(processStatusPtr);
			processStatusPtr->dataLength = firstByteLow;

			for (uint8_t i = 0; i < processStatusPtr->dataLength; i++)
			{
				processStatusPtr->data[i] = processStatusPtr->dataBuffer[i + 2];
				cmd_Response.data[i] = processStatusPtr->dataBuffer[i + 2];
			}

			cmd_Response.needResp = RT_TRUE;

			if (RT_NULL != processStatusPtr->tpSendData->Lin_TpReceiveEndFunc)
			{
				processStatusPtr->tpSendData->Lin_TpReceiveEndFunc(0, processStatusPtr->dataLength);
			}
		}
	}
	else if (0x10 == pci)
	{
		/*response is ff*/
		uint16_t dataLength = firstByteLow;
		dataLength <<= 8;
		dataLength |= processStatusPtr->dataBuffer[2];

		processStatusPtr->tpTimer = LIN_TP_N_CR;
		processStatusPtr->time = Tm_ResetTimer100us32bit();
		processStatusPtr->dataLength = dataLength;
		processStatusPtr->dataIndex = 0;
		processStatusPtr->frameCounter = 1;
		processStatusPtr->nad = processStatusPtr->dataBuffer[0];

		for (uint8_t i = 0; i < 5; i++)
		{
			processStatusPtr->data[i] = processStatusPtr->dataBuffer[i + 3];
			processStatusPtr->dataIndex++;
		}
	}
	else if (0x20 == pci)
	{
		/*response is cf*/
		if ((processStatusPtr->frameCounter == firstByteLow) && (processStatusPtr->nad != processStatusPtr->dataBuffer[0]))
		{
			for (uint8_t i = 0; ((i < 6) && (processStatusPtr->dataIndex < processStatusPtr->dataLength)); i++)
			{
				processStatusPtr->data[i + processStatusPtr->dataIndex] = processStatusPtr->dataBuffer[i + 2];
				processStatusPtr->dataIndex++;
			}

			/*response end*/
			if (processStatusPtr->dataIndex == processStatusPtr->dataLength)
			{
				if (RT_NULL != processStatusPtr->tpSendData->Lin_TpReceiveEndFunc)
				{
					processStatusPtr->tpSendData->Lin_TpReceiveEndFunc(0, processStatusPtr->dataLength);
				}

				Lin_ResetProcess(processStatusPtr);
			}
			else
			{
				processStatusPtr->frameCounter++;

				if (0x0F < processStatusPtr->frameCounter)
				{
					processStatusPtr->frameCounter = 0;
				}

				processStatusPtr->tpTimer = LIN_TP_N_CR;
				processStatusPtr->time = Tm_ResetTimer100us32bit();
			}
		}
		else
		{
			/*wrong frame counter or wrong nad*/
			if (RT_NULL != processStatusPtr->tpSendData->Lin_TpReceiveEndFunc)
			{
				processStatusPtr->tpSendData->Lin_TpReceiveEndFunc(1, processStatusPtr->dataLength);
			}

			Lin_ResetProcess(processStatusPtr);
		}
	}
	else
	{
		/*not except pci*/
		if (RT_NULL != processStatusPtr->tpSendData->Lin_TpReceiveEndFunc)
		{
			processStatusPtr->tpSendData->Lin_TpReceiveEndFunc(1, processStatusPtr->dataLength);
		}

		Lin_ResetProcess(processStatusPtr);
	}
}
