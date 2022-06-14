/*
 * Lin.h
 *
 *  Created on: 2022年5月14日
 *      Author: cheng
 */

#ifndef LIN_H
#define LIN_H

#include "main.h"
#include "usart.h"

#define LIN_TIMEOUT_10MS                           10
#define LIN_DIAG_MESSAGE_INDEX_MAX                 12

typedef void (*Lin_TpSendCbkFuncType)(uint8_t result);
typedef void (*Lin_TpReceiveEndFuncType)(uint8_t result, uint16_t dataLen);

typedef struct
{
    Lin_TpSendCbkFuncType Lin_TpSendCbkFunc;
    Lin_TpReceiveEndFuncType Lin_TpReceiveEndFunc;
    const uint8_t* data;
    uint8_t dataLen;
	uint8_t nad;
	uint8_t needResp;
} Lin_TpSendDataType;

typedef enum
{
	LIN_STATUS_IDLE = 0u,
	LIN_STATUS_PROESSING
} Lin_MainStatusType;

typedef enum
{
	LIN_TP_N_AS = 0u,
	LIN_TP_N_CS,
	LIN_TP_N_CR
} Lin_TpTimerType;

typedef enum
{
	LIN_SUB_STATUS_IDLE = 0u,
	LIN_SUB_STATUS_SENDING,
	LIN_SUB_STATUS_PRE_RECEIVE,
	LIN_SUB_STATUS_WAIT_RECEIVE,
	LIN_SUB_STATUS_RECEIVING
} Lin_SubStatusType;

typedef struct
{
	Lin_MainStatusType mainStatus;
	Lin_SubStatusType subStatus;
	Lin_TpTimerType tpTimer;
	uint32_t time;
	uint8_t* data;
	uint8_t dataBuffer[8];
	uint16_t dataLength;
	uint16_t dataIndex;
	uint8_t needResp;
	uint8_t frameCounter;
	uint8_t nad;
	const Lin_TpSendDataType* tpSendData;
} Lin_ProessStatusType;

extern Lin_ProessStatusType Lin_ProessStatus;

#define BIT(a,b)                 ((a >> b) & 0x01u)
#define LIN_SYNC_BYTE            0x55u
#define LIN_DATA_SIZE            8u

#define LIN_PID_20H    			 0x20u
#define LIN_PID_21H              0x21u

typedef struct
{
	uint8_t length;
	uint8_t sync;
	uint8_t id;
	uint8_t data[LIN_DATA_SIZE];
}Lin_FrameInfoType;

void Lin_MainFunction(void);
void Lin_Init(void);
void Lin_TpSendData(const Lin_TpSendDataType* Lin_TpSendData);
void cmd_LinStart(uint8_t* data, uint8_t len);
void cmd_LinStop(uint8_t* data, uint8_t len);
#endif /* LIN_H */
