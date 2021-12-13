/*
 * CCS811.c
 *
 *  Created on: Dec 6, 2021
 *      Author: Onur Karaman
 */

#include "CCS811.h"

static uint8_t MEAS_MODE_REG = 0x01;
static uint8_t Mode0 = 0x00;
static uint8_t Mode1 = 0x10;
static uint8_t Mode2 = 0x20;
static uint8_t Mode3 = 0x30;
static uint8_t Mode4 = 0x40;

static void CCS811_SoftwareReset(void)
{
	uint8_t txData[5] = {0};

	/* Software Reset Register Address */
	txData[0] = 0xFF;

	/* Reset Sequence*/
	txData[1] = 0x11;
	txData[2] = 0xE5;
	txData[3] = 0x72;
	txData[4] = 0x8A;

	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), txData, 5, HAL_MAX_DELAY);

	HAL_Delay(500);
}

static void CCS811_ApplicationStart(void)
{
	/* Application Start Register Address */
	 uint8_t txData = 0xF4;

	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &txData, 1, HAL_MAX_DELAY);

	HAL_Delay(500);
}

static uint8_t CCS811_ReadStatus(void)
{
	/* Status Register Address */
	uint8_t txData = 0x00;

	/* Buffer to store return value */
	uint8_t rxData = 0;

	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &txData, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, (CCS822_DEVICE_ADDR<<1), &rxData, 1, HAL_MAX_DELAY);

	return rxData;
}

static void CCS811_SetMeasureMode(int mode)
{
	/* Application Start */
	CCS811_ApplicationStart();

	/* Check if firmware is in application mode */
	uint8_t firmwareMode = 0b10000000;
	uint8_t status = CCS811_ReadStatus();
	if(!(status & firmwareMode)) while(1);

	switch(mode)
	{
	case 1:
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &MEAS_MODE_REG, 1, HAL_MAX_DELAY);
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &Mode1, 1, HAL_MAX_DELAY);
		break;
	case 2:
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &MEAS_MODE_REG, 1, HAL_MAX_DELAY);
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &Mode2, 1, HAL_MAX_DELAY);
	case 3:
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &MEAS_MODE_REG, 1, HAL_MAX_DELAY);
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &Mode3, 1, HAL_MAX_DELAY);
	case 4:
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &MEAS_MODE_REG, 1, HAL_MAX_DELAY);
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &Mode4, 1, HAL_MAX_DELAY);
	default:
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &MEAS_MODE_REG, 1, HAL_MAX_DELAY);
		HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &Mode1, 1, HAL_MAX_DELAY);
	}
}

static void CCS811_GetAlgorithmResultsData(void)
{
	/* Check if the data ready */
	uint8_t dataReady = 0b00001000;
	uint8_t status = 0;
	while(!(status & dataReady))
	{
		status = CCS811_ReadStatus();
		HAL_Delay(10);
	}

	uint8_t txData = 0x02;
	uint8_t rxData[8] = {0};

	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &txData, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, (CCS822_DEVICE_ADDR<<1), rxData, 8, HAL_MAX_DELAY);

	CCS811_AlgorithmResults._eCO2_HighByte = rxData[0];
	CCS811_AlgorithmResults._eCO2_LowByte = rxData[1];
	CCS811_AlgorithmResults._eTVOC_HighByte = rxData[2];
	CCS811_AlgorithmResults._eTVOC_LowByte = rxData[3];
	CCS811_AlgorithmResults._STATUS = rxData[4];
	CCS811_AlgorithmResults._ERROR_ID = rxData[5];
	CCS811_AlgorithmResults._RAW_DATA = (rxData[6] << 8) | (rxData[7]);
}

void CCS811_IdleMode(void)
{
	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &MEAS_MODE_REG, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &Mode0, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}

void CCS811_WakeUp(int mode)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_Delay(50);
	CCS811_SetMeasureMode(mode);
}

void CCS811_ReadData(void)
{
	CCS811_GetAlgorithmResultsData();
}

void CCS822_Init(int mode)
{
	/* Software Reset */
	CCS811_SoftwareReset();

	/* Check Hardware ID */
	uint8_t rxData = 0;
	uint8_t txData = 0x20;
	HAL_I2C_Master_Transmit(&hi2c1, (CCS822_DEVICE_ADDR<<1), &txData, 1, HAL_MAX_DELAY);
	HAL_I2C_Master_Receive(&hi2c1, (CCS822_DEVICE_ADDR<<1), &rxData, 1, HAL_MAX_DELAY);
	if(rxData != 0x81) while(1);

	/* Check if the app is valid */
	uint8_t appValid = 0b00010000;
	uint8_t status = CCS811_ReadStatus();
	if(!(status & appValid)) while(1);

	/* Set Drive Mode (as interrupt disabled) */
	CCS811_SetMeasureMode(mode);
	HAL_Delay(4000);
}
