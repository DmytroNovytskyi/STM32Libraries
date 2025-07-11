/**
 * @brief Implementation of KY-040 Rotary Encoder library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#include "KY-040.h"

typedef struct {
	KY_040_HandleTypeDef *handle;
	bool pressed;
} KY_040_Device;

static KY_040_Device devices[MAX_DEVICES] = { 0 };

HAL_StatusTypeDef KY_040_Init(KY_040_HandleTypeDef *handle) {
	HAL_StatusTypeDef status = HAL_TIM_Encoder_Start_IT(handle->htim, TIM_CHANNEL_1);
	if (status != HAL_OK) {
		return status;
	}

	for (int i = 0; i < MAX_DEVICES; i++) {
		if (devices[i].handle == NULL) {
			devices[i].handle = handle;
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}

int16_t KY_040_GetEncoderValue(KY_040_HandleTypeDef *handle) {
	int16_t encoderValue = (int16_t) __HAL_TIM_GET_COUNTER(handle->htim);
	__HAL_TIM_SET_COUNTER(handle->htim, 0);
	return encoderValue;
}

bool KY_040_IsPressed(KY_040_HandleTypeDef *handle) {
	for (int i = 0; i < MAX_DEVICES; i++) {
		if (devices[i].handle == handle) {
			return devices[i].pressed;
		}
	}
	return false;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	for (int i = 0; i < MAX_DEVICES; i++) {
		if (devices[i].handle->swPin == GPIO_Pin) {
			devices[i].pressed = !HAL_GPIO_ReadPin(devices[i].handle->swPort, devices[i].handle->swPin);
			return;
		}
	}
}

