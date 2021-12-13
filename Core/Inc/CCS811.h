/*
 * CCS811.h
 *
 *  Created on: Dec 6, 2021
 *      Author: Onur Karaman
 */

#ifndef INC_CCS811_H_
#define INC_CCS811_H_

#include "i2c.h"
#include "main.h"

#define CCS822_DEVICE_ADDR  0x5A

void CCS822_Init(int mode);
void CCS811_ReadData(void);
void CCS811_IdleMode(void);
void CCS811_WakeUp(int mode);

struct CCS811_Alg
{
	uint8_t _eCO2_HighByte;
	uint8_t _eCO2_LowByte;
	uint8_t _eTVOC_HighByte;
	uint8_t _eTVOC_LowByte;
	uint8_t _STATUS;
	uint8_t _ERROR_ID;
	uint16_t _RAW_DATA;

}CCS811_AlgorithmResults;

#endif /* INC_CCS811_H_ */
