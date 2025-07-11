/**
 * @brief Implementation of AT24Cxx library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.1
 */

#include "AT24Cxx.h"
#include <string.h>

//Formats 16-bit memory address as two bytes in buffer
static void AT24CXX_FormatMemoryAddress(uint16_t address, uint8_t *buffer);

HAL_StatusTypeDef AT24CXX_IsDeviceReady(AT24CXX_HandleTypeDef *handle, uint32_t trials, uint32_t timeout) {
	if (handle == NULL) {
		return HAL_ERROR;
	}
	return HAL_I2C_IsDeviceReady(handle->hi2c, handle->deviceAddress, trials, timeout);
}

HAL_StatusTypeDef AT24CXX_Read(AT24CXX_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size,
		uint32_t timeout) {
	if (handle == NULL || data == NULL || size == 0 || (address + size - 1) > (handle->memorySizeKbit * 1024 / 8 - 1)) {
		return HAL_ERROR;
	}

	uint8_t addressBytes[2];
	HAL_StatusTypeDef status;
	AT24CXX_FormatMemoryAddress(address, addressBytes);
	status = HAL_I2C_Master_Transmit(handle->hi2c, handle->deviceAddress, addressBytes, 2, timeout);
	if (status != HAL_OK) {
		return status;
	}

	return HAL_I2C_Master_Receive(handle->hi2c, handle->deviceAddress, data, size, timeout);
}

HAL_StatusTypeDef AT24CXX_Write(AT24CXX_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size,
		uint32_t timeout) {
	if (handle == NULL || data == NULL || size == 0 || (address + size - 1) > (handle->memorySizeKbit * 1024 / 8 - 1)) {
		return HAL_ERROR;
	}

	uint16_t bytesRemaining = size;
	uint16_t currentAddress = address;
	uint8_t *currentDataPtr = data;
	HAL_StatusTypeDef status = HAL_OK;

	while (bytesRemaining > 0)
	{
		uint16_t pageOffset = currentAddress % handle->pageSize;
		uint16_t bytesToWriteInPage = handle->pageSize - pageOffset;

		if (bytesToWriteInPage > bytesRemaining) {
			bytesToWriteInPage = bytesRemaining;
		}

		uint8_t txBuffer[2 + bytesToWriteInPage];
		AT24CXX_FormatMemoryAddress(currentAddress, txBuffer);
		memcpy(&txBuffer[2], currentDataPtr, bytesToWriteInPage);

		status = HAL_I2C_Master_Transmit(handle->hi2c, handle->deviceAddress, txBuffer, bytesToWriteInPage + 2,
				timeout);
		if (status != HAL_OK) {
			return status;
		}

		HAL_Delay(5);

		bytesRemaining -= bytesToWriteInPage;
		currentAddress += bytesToWriteInPage;
		currentDataPtr += bytesToWriteInPage;
	}

	return HAL_OK;
}

HAL_StatusTypeDef AT24CXX_Erase(AT24CXX_HandleTypeDef *handle, uint32_t timeout)
{
	if (handle == NULL)
	{
		return HAL_ERROR;
	}

	uint8_t pageBuffer[handle->pageSize];
	memset(pageBuffer, 0x00, handle->pageSize);

	HAL_StatusTypeDef status = HAL_OK;
	for (uint32_t currentAddress = 0; currentAddress <= (handle->memorySizeKbit * 1024 / 8 - 1);
			currentAddress += handle->pageSize) {
		status = AT24CXX_Write(handle, currentAddress, pageBuffer, handle->pageSize, timeout);
		if (status != HAL_OK) {
			return status;
		}
	}

	return HAL_OK;
}

static void AT24CXX_FormatMemoryAddress(uint16_t address, uint8_t *buffer) {
	buffer[0] = (address >> 8) & 0xFF;
	buffer[1] = address & 0xFF;
}
