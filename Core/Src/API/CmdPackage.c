#include <string.h>
#include "main.h"
#include "API/CRC.h"
#include "API/CmdPackage.h"

static CmdHdr cmdHeader = {
	.ver = 0x01,
	.magicNum = 0xaabb,
};

#define CRC_LEN (4)
#define MAX_CMD_LEN (255)
#define MAX_DATA_LEN (MAX_CMD_LEN - sizeof(cmdHeader) - CRC_LEN)

int Cmd_pack(uint8_t cmdId, uint8_t *pData, uint8_t dataLen, uint8_t *pCmd, uint8_t *pCmdLen)
{
	if (!pCmd || !pCmdLen) {
		return -1;
	}
	if (0 != dataLen && !pData) {
		return -1;
	}
	if (MAX_DATA_LEN < dataLen) {
		return -1;
	}

	cmdHeader.addr = 0x10;
	cmdHeader.cmdId = cmdId;
	cmdHeader.dataLen = dataLen;

	memcpy(pCmd, &cmdHeader, sizeof(CmdHdr));
	memcpy(pCmd + sizeof(CmdHdr), pData, dataLen);
	uint32_t crc = CRC32_calculate(pCmd, sizeof(CmdHdr) + dataLen);
	memcpy(pCmd + sizeof(CmdHdr) + dataLen, &crc, CRC_LEN);
	*pCmdLen = sizeof(CmdHdr) + dataLen + CRC_LEN;

	return 0;
}

int Cmd_unpack(uint8_t *pCmdId, uint8_t *pCmd, uint8_t cmdLen, uint8_t *pData, uint8_t *pDataLen)
{
	if (!pCmdId || !pCmd || sizeof(CmdHdr) + CRC_LEN > cmdLen || !pData || !pDataLen) {
		return -1;
	}

	uint32_t crcCalc = CRC32_calculate(pCmd, cmdLen - CRC_LEN);
	uint32_t crcParse;
	memcpy(&crcParse, pCmd + cmdLen - CRC_LEN, CRC_LEN);
	if (crcCalc != crcParse) {
		return -2;//CRC check error
	}

	CmdHdr hdr;
	memcpy(&hdr, pCmd, sizeof(CmdHdr));
	if (hdr.magicNum != cmdHeader.magicNum) {
		return -3;//check magic number error
	}

	*pCmdId = hdr.cmdId;
	memcpy(pData, pCmd + sizeof(CmdHdr), hdr.dataLen);
	*pDataLen = hdr.dataLen;

	return 0;
}
