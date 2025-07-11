/**
 * @brief STM32 HAL-based KY-040 Rotary Encoder library
 *
 * This library provides an interface for reading the KY-040 Rotary Encoder
 * and its integrated button.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef KY_040_H
#define KY_040_H

#include <stdbool.h>
#include "stm32f1xx_hal.h"

/**
 * @brief Maximum number of KY-040 devices that can be managed by the library.
 */
#define MAX_DEVICES 1

/**
 * @brief KY-040 Rotary Encoder handle structure
 *
 * This structure holds the configuration for a KY-040 Rotary Encoder device.
 *
 * Fields:
 * - htim:   Pointer to the HAL TIM handle configured for encoder mode.
 * - swPort: Pointer to the GPIO port connected to the switch (SW) pin.
 * - swPin:  The GPIO pin number connected to the switch (SW) pin.
 */
typedef struct {
	TIM_HandleTypeDef *htim;
	GPIO_TypeDef *swPort;
	uint16_t swPin;
} KY_040_HandleTypeDef;

/**
 * @brief Initializes a KY-040 Rotary Encoder handle.
 *
 * Configures the KY_040_HandleTypeDef structure and starts the timer in encoder mode.
 *
 * @param handle Pointer to the KY_040_HandleTypeDef structure to initialize.
 * @return       HAL_StatusTypeDef HAL_OK if initialization is successful, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef KY_040_Init(KY_040_HandleTypeDef *handle);

/**
 * @brief Gets the current encoder value and resets the counter.
 *
 * Reads the current value from the encoder timer and resets the timer counter to zero.
 *
 * @param handle Pointer to the KY_040_HandleTypeDef structure.
 * @return       int16_t The encoder value (positive for clockwise, negative for counter-clockwise).
 */
int16_t KY_040_GetEncoderValue(KY_040_HandleTypeDef *handle);

/**
 * @brief Checks if the KY-040 button is pressed.
 *
 * Reads the state of the button.
 *
 * @param handle Pointer to the KY_040_HandleTypeDef structure.
 * @return       bool true if the button is pressed, false otherwise.
 */
bool KY_040_IsPressed(KY_040_HandleTypeDef *handle);

#endif //KY_040_H
