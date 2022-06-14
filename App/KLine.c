/*
 * KLine.c
 *
 *  Created on: May 16, 2022
 *      Author: cheng
 */

#include "main.h"
#include "usart.h"
#include "KLine.h"
#include "usbd_cdc_if.h"

typedef struct
{
	uint16_t respLen;
	uint16_t dataPos;
	uint8_t data[KLINE_RESPONSE_LENGTH_MAX];
} KLine_ResponseType;

static KLine_ResponseType KLine_Response;

void KLine_Init(void)
{
	KLine_Response.respLen = 0;
	KLine_Response.dataPos = 0;
}

void KLine_RxIndication(uint8_t* data, uint16_t length)
{
	KLine_Response.data[0] = 0xA0;
	KLine_Response.respLen = (length + 1);
	while (length--)
	{
		KLine_Response.data[1 + length] = data[length];
	}
}

void KLine_MainFunction(void)
{
	if (KLine_Response.dataPos != KLine_Response.respLen)
	{
		if (USBD_OK == CDC_Transmit_FS(&(KLine_Response.data[KLine_Response.dataPos]), 64))
		{
			if (64 < KLine_Response.respLen)
			{
				KLine_Response.dataPos += 64u;
			}
			else
			{
				KLine_Response.respLen = 0;
				KLine_Response.dataPos = 0;
			}
		}
	}
}
