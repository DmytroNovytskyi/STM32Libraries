/**
 * @brief STM32 HAL-based AT24Cxx library
 *
 * This library provides a simple abstraction for controlling AT24Cxx EEPROM chip using I2C protocol.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.1
 */

#ifndef AT24CXX_H
#define AT24CXX_H

#include "stm32f1xx_hal.h"

/**
 * @brief Base 7-bit I2C address for AT24Cxx series EEPROMs.
 * This is the fixed part of the device address before shifting.
 * The full address will also include A0, A1, A2 pins.
 */
#define AT24CXX_BASE_ADDRESS 0x50

/**
 * @brief AT24Cxx EEPROM handle structure
 *
 * This structure holds the configuration and state for an AT24Cxx EEPROM device.
 *
 * Fields:
 * - hi2c:           Pointer to the HAL I2C handle associated with this EEPROM.
 * - deviceAddress:  The full 8-bit I2C device address, including the R/W bit space
 * 				     (7-bit address shifted left by 1). Example: A2 = 1, A1 = 0, A0 = 0 -> 0xA8
 * - memorySizeKbit: The total memory size of the EEPROM chip in kilobits (e.g., 256 for AT24C256/24LC256).
 * - pageSize:       The write page size in bytes (e.g., 64 for AT24C256/24LC256).
 */
typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t deviceAddress;
	uint16_t memorySizeKbit;
	uint16_t pageSize;
} AT24CXX_HandleTypeDef;

/**
 * @brief Checks if the AT24Cxx EEPROM device is ready for communication.
 *
 * Sends a dummy byte to the EEPROM to check for an Acknowledge (ACK) from the device.
 * This can be used after a write operation to determine when the internal write cycle is complete.
 *
 * @param handle  Pointer to the AT24CXX_HandleTypeDef structure.
 * @param trials  Number of times to try checking the device readiness.
 * @param timeout Timeout duration in milliseconds for each trial.
 * @return 		  HAL_StatusTypeDef HAL_OK if the device is ready, HAL_TIMEOUT or HAL_ERROR otherwise.
 */
HAL_StatusTypeDef AT24CXX_IsDeviceReady(AT24CXX_HandleTypeDef *handle, uint32_t trials, uint32_t timeout);

/**
 * @brief Reads a specified number of bytes from the AT24Cxx EEPROM.
 *
 * This function reads 'size' bytes starting from 'address' into the 'data' buffer.
 * It handles continuous reads across page boundaries automatically.
 *
 * @param handle  Pointer to the AT24CXX_HandleTypeDef structure.
 * @param address The starting memory address within the EEPROM (0-indexed).
 * @param data    Pointer to the buffer where the read data will be stored.
 * @param size    The number of bytes to read.
 * @param timeout Timeout duration in milliseconds for the I2C operation.
 * @return 		  HAL_StatusTypeDef HAL_OK if data is read successfully, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef AT24CXX_Read(AT24CXX_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size,
		uint32_t timeout);

/**
 * @brief Writes a specified number of bytes to the AT24Cxx EEPROM.
 *
 * This function writes 'size' bytes from the 'data' buffer starting at 'address'.
 * It automatically handles page boundary crossings by splitting the write into multiple page writes.
 * A delay of 5 ms is inserted after each page write to allow the EEPROM's internal write cycle to complete.
 *
 * @param handle  Pointer to the AT24CXX_HandleTypeDef structure.
 * @param address The starting memory address within the EEPROM (0-indexed).
 * @param data    Pointer to the buffer containing data to be written.
 * @param size    The number of bytes to write.
 * @param timeout Timeout duration in milliseconds for each I2C page write operation.
 * @return 		  HAL_StatusTypeDef HAL_OK if data is written successfully, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef AT24CXX_Write(AT24CXX_HandleTypeDef *handle, uint16_t address, uint8_t *data, uint16_t size,
		uint32_t timeout);

/**
 * @brief Erases the entire AT24Cxx EEPROM memory by writing zeros (0x00).
 *
 * This function iterates through all memory addresses and writes 0x00 to each page.
 * It utilizes the page write capability for efficiency and handles internal write delays.
 * A delay of 5 ms is inserted after each page write to allow the EEPROM's internal write cycle to complete.
 *
 * @param handle  Pointer to the AT24CXX_HandleTypeDef structure.
 * @param timeout Timeout duration in milliseconds for each I2C page write operation.
 * @return 		  HAL_StatusTypeDef HAL_OK if the memory is successfully erased, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef AT24CXX_Erase(AT24CXX_HandleTypeDef *handle, uint32_t timeout);

#endif // AT24CXX_H
