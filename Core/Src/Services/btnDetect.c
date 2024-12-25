#include "main.h"
#include "API/CmdPackage.h"
#include "API/CmdProc.h"

extern I2C_HandleTypeDef hi2c2;

#define BUF_SIZE (50)
static uint8_t arrMaRx[BUF_SIZE];
static uint8_t arrMaTx[BUF_SIZE];
static uint8_t rxCnt = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_PIN_13 == GPIO_Pin) {
        if (GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOC, GPIO_Pin))
        {
        	uint8_t arrD[] = {50};//PWM 50%
        	uint8_t txLen;
        	if (Cmd_pack(CMD_W_FAN_PWM, arrD, 1, arrMaTx, &txLen)) {
        		return;
        	}

			if (HAL_OK != HAL_I2C_Master_Seq_Transmit_DMA(&hi2c2, 0x10 << 1, arrMaTx, txLen, I2C_FIRST_FRAME)) {
				return;
			}
        }
    }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if (I2C2 == hi2c->Instance) {
		rxCnt = 0;
    	if (HAL_OK != HAL_I2C_Master_Seq_Receive_DMA(&hi2c2, 0x10 << 1, arrMaRx, sizeof(CmdHdr), I2C_NEXT_FRAME)) {
    		return;
    	}
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	static uint8_t dataLen = 0;
	if (I2C2 == hi2c->Instance) {
		if (0 == rxCnt) {
			rxCnt += sizeof(CmdHdr);
			CmdHdr *pHdr = (CmdHdr*)arrMaRx;
			dataLen = pHdr->dataLen;

			if (HAL_OK != HAL_I2C_Master_Seq_Receive_DMA(hi2c, 0x10 << 1, arrMaRx + sizeof(CmdHdr), dataLen + CRC_LEN, I2C_LAST_FRAME)) {
				return;
			}
		}
		else {
			rxCnt += (dataLen + CRC_LEN);
			//read finished

			uint8_t cmdId;
			uint8_t arrRes[BUF_SIZE] = {0};
			uint8_t resLen;

			if (Cmd_unpack(&cmdId, arrMaRx, rxCnt, arrRes, &resLen)) {
				return;
			}

			if (0 != arrRes[0]) {
				//cmd error
				return;
			}

		}
	}
}
