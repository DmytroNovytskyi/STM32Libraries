/**
 * @brief STM32 HAL-based library for KY-023 joystick module.
 * The ADC_Regular_ConversionMode must be enabled in the .ioc file for proper code generation.
 * It may be necessary to calibrate the values for the specific joystick used,
 * as there may be slight variations in the output.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef KY_023_H
#define KY_023_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <math.h>

/**
 * @brief Minimal analog tilt value treated as noise (joystick rest state)
 *
 * This defines the threshold below which tilt values are considered as drift or noise.
 * Values within this range will be ignored during processing.
 */
#define KY_023_DRIFT_DEADZONE 0.08

/**
 * @brief Analog value threshold above which the joystick button is considered pressed
 *
 * This threshold is used to determine whether the joystick button (SW) is pressed or not.
 * The value of the button channel is compared to this threshold.
 */
#define KY_023_PRESSED_THRESHOLD 0.90

/**
 * @brief Joystick configuration: associated ADC and input channels
 *
 * This structure holds configuration for the joystick device, including:
 * - A pointer to the ADC handler.
 * - The channels for reading the X-axis, Y-axis, and the switch (button) state.
 */
typedef struct {
	ADC_HandleTypeDef *hadc;
	uint32_t channelX;
	uint32_t channelY;
	uint32_t channelSW;
} KY_023_Device;

/**
 * @brief Joystick current state: filtered tilt and pressed state
 *
 * This structure represents the current state of the joystick, which includes:
 * - Filtered X and Y tilt values (normalized between -1.0 and 1.0).
 * - The state of the joystick button, which can either be pressed or not.
 */
typedef struct {
	float xTilt;
	float yTilt;
	bool pressed;
} KY_023_State;

KY_023_State KY_023_ReadState(KY_023_Device *device);

#endif //KY_023_H
