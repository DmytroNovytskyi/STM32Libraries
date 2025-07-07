/**
 * @brief STM32 HAL-based ADC library
 *
 * This library provides a simple abstraction for using ADC of STM32 through the HAL interface.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef ADC_H
#define ADC_H

#include "stm32f1xx_hal.h"

/**
 * @brief Calibrates the ADC
 *
 * This function initiates and waits for the ADC calibration to complete.
 *
 * @param hadc Pointer to the ADC handle
 */
void ADC_Calibrate(ADC_HandleTypeDef* hadc);

/**
 * @brief Reads a value from the specified ADC channel
 *
 * This function configures the given channel for a regular conversion and reads the ADC value.
 *
 * @param hadc Pointer to the ADC handle
 * @param channel The ADC channel to read (e.g., ADC_CHANNEL_0)
 * @return The 16-bit digital value read from the 12-bit ADC
 */
uint16_t ADC_ReadValue(ADC_HandleTypeDef* hadc, uint32_t channel);

#endif // ADC_H
