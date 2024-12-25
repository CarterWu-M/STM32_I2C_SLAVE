#ifndef INC_API_CMDPACKAGE_H_
#define INC_API_CMDPACKAGE_H_

typedef struct {
	uint8_t addr;
	const uint8_t ver;
	uint8_t cmdId;
	uint8_t dataLen;
	const uint16_t magicNum;
} CmdHdr;

// cmd = | header (6 bytes) | data (dataLen bytes) | CRC32 (4 bytes) |
#define CRC_LEN (4)

int Cmd_pack(uint8_t cmdId, uint8_t *pData, uint8_t dataLen, uint8_t *pCmd, uint8_t *pCmdLen);
int Cmd_unpack(uint8_t *pCmdId, uint8_t *pCmd, uint8_t cmdLen, uint8_t *pData, uint8_t *pDataLen);

#endif /* INC_API_CMDPACKAGE_H_ */
