/**
 * @brief Implementation of KY-023 joystick module library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#include "KY-023.h"

//Applies deadzone filtering and normalizes joystick output to [-1, 1]
static float KY_023_ApplyFilter(float value) {
	if (fabs(value) < KY_023_DRIFT_DEADZONE) {
		return 0.0f; // Ignore small drift
	}

	//Convert cut off range to full
	return (value > 0) ? (value - KY_023_DRIFT_DEADZONE) / (1 - KY_023_DRIFT_DEADZONE) :
							(value + KY_023_DRIFT_DEADZONE) / (1 - KY_023_DRIFT_DEADZONE);
}

//Reads and filters analog value from specified ADC channel
static float KY_023_ReadChannel(ADC_HandleTypeDef *hadc, uint32_t channel) {
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

	//Apply filter and return
	return KY_023_ApplyFilter((value / 2048.0 - 1) * -1.0);
}

//Reads joystick tilt (X/Y) and press state
KY_023_State KY_023_ReadState(KY_023_Device *device) {
	KY_023_State state;
	state.xTilt = KY_023_ReadChannel(device->hadc, device->channelX);
	state.yTilt = KY_023_ReadChannel(device->hadc, device->channelY);
	state.pressed = KY_023_ReadChannel(device->hadc, device->channelSW) > KY_023_PRESSED_THRESHOLD;
	return state;
}
