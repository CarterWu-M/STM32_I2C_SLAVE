
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "Services/I2C_slave.h"
#include "API/CmdPackage.h"
#include "API/CmdProc.h"

extern I2C_HandleTypeDef hi2c1;

#define BUF_SIZE (50)
static uint8_t arrSlRx[BUF_SIZE];
static uint8_t arrSlTx[BUF_SIZE];
static bool isRxDone = false;
static uint8_t rxCnt = 0;
static uint8_t txCnt = 0;

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if (I2C1 == hi2c->Instance) {
		if (I2C_DIRECTION_TRANSMIT == TransferDirection) {
			rxCnt = 0;
			if (HAL_OK != HAL_I2C_Slave_Seq_Receive_DMA(hi2c, arrSlRx, sizeof(CmdHdr), I2C_FIRST_FRAME)) {
				return;
			}
		}
		else {//I2C_DIRECTION_RECEIVE
			if (HAL_OK != HAL_I2C_Slave_Seq_Transmit_DMA(hi2c, arrSlTx, txCnt, I2C_FIRST_AND_LAST_FRAME)) {
				return;
			}
		}
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	//reserve
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	static uint8_t dataLen = 0;

	if (0 == rxCnt) {
		rxCnt += sizeof(CmdHdr);
		CmdHdr *pHdr = (CmdHdr*)arrSlRx;
		dataLen = pHdr->dataLen;

		if (HAL_OK != HAL_I2C_Slave_Seq_Receive_DMA(hi2c, arrSlRx + sizeof(CmdHdr), dataLen + CRC_LEN, I2C_LAST_FRAME)) {
			return;
		}
	}
	else {
		rxCnt += (dataLen + CRC_LEN);
		isRxDone = true;
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if (I2C1 == hi2c->Instance) {
		if (HAL_I2C_ERROR_AF == HAL_I2C_GetError(hi2c))//AF error
		{
			__HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);//clear AF flag
		}
		HAL_I2C_EnableListen_IT(hi2c);
	}
}


int I2CSlave_init(void)
{
	hi2c1.Init.OwnAddress1 = 0x10 << 1;

	if (HAL_OK != HAL_I2C_Init(&hi2c1)) {
		return -1;
	}

	HAL_I2C_EnableListen_IT(&hi2c1);
	return 0;
}

int I2CSlave_task(void)
{
	int ret = 0;

	if (isRxDone) {
		isRxDone = false;

		uint8_t cmdId;
		uint8_t arrReq[BUF_SIZE] = {0};
		uint8_t arrRes[BUF_SIZE] = {0};
		uint8_t reqLen;
		uint8_t resLen;

		if (Cmd_unpack(&cmdId, arrSlRx, rxCnt, arrReq, &reqLen)) {
			return -1;
		}

		ret = Cmd_process(cmdId, arrReq, &reqLen, arrRes, &resLen);

		if (resLen) {
			// for the read command response
			if (Cmd_pack(cmdId, arrRes, resLen, arrSlTx, &txCnt)) {
				return -1;
			}
		}
		else {
			// for the write command response
			if (Cmd_pack(cmdId, (uint8_t*)&ret, 1, arrSlTx, &txCnt)) {
				return -1;
			}
		}

		memset(arrSlRx, 0, sizeof(arrSlRx));
	}

	return 0;
}
