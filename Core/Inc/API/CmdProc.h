#ifndef INC_API_CMDPROC_H_
#define INC_API_CMDPROC_H_

//command ID
#define CMD_R_FAN_SPEED (0x01)
#define CMD_W_FAN_PWM   (0x02)

int Cmd_process(uint8_t cmdId, uint8_t *parrReq, uint8_t *pReqLen, uint8_t *parrRes, uint8_t *pResLen);

#endif /* INC_API_CMDPROC_H_ */
