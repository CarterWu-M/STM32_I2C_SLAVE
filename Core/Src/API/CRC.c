#include <string.h>
#include "main.h"
#include "API/CRC.h"

extern CRC_HandleTypeDef hcrc;

uint32_t CRC32_calculate(uint8_t *pData, uint16_t len)
{
	if (!pData || !len) {
		return -1;
	}

    uint32_t crcResult;

    // Check if length is a multiple of 4
    uint16_t words = len / 4; // Full 32-bit words
    uint16_t remainder = len % 4;

    // Process full 32-bit words
    crcResult = HAL_CRC_Calculate(&hcrc, (uint32_t *)pData, words);

    // Handle remaining bytes (if any)
    if (remainder > 0) {
        uint32_t lastWord = 0;
        memcpy(&lastWord, &pData[words * 4], remainder); // Copy remaining bytes
        crcResult = HAL_CRC_Accumulate(&hcrc, &lastWord, 1); // Accumulate remainder
    }

    return crcResult;
}
