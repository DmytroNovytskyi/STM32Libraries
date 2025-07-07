/**
 * @brief Implementation of ADC library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#include "ADC.h"

void ADC_Calibrate(ADC_HandleTypeDef *hadc) {
	while (HAL_ADCEx_Calibration_Start(hadc) != HAL_OK)
		;
}

uint16_t ADC_ReadValue(ADC_HandleTypeDef *hadc, uint32_t channel) {
	uint16_t value;

	//Configure channel
	ADC_ChannelConfTypeDef channelConfig = { 0 };
	channelConfig.Channel = channel;
	channelConfig.Rank = ADC_REGULAR_RANK_1;
	channelConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	HAL_ADC_ConfigChannel(hadc, &channelConfig);

	//Poll value
	HAL_ADC_Start(hadc);
	HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
	value = HAL_ADC_GetValue(hadc);
	HAL_ADC_Stop(hadc);

	return value;
}
