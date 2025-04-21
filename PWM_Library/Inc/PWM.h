/**
 * @brief STM32 HAL-based PWM library
 *
 * This library provides a simple abstraction for configuring and controlling PWM output
 * using STM32 timers through the HAL interface.
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#ifndef PWM_H
#define PWM_H

#include <stdbool.h>
#include <math.h>
#include "stm32f1xx_hal.h"

/**
 * @brief PWM timer handle
 *
 * Fields:
 * - htim:      Pointer to HAL timer handle
 * - frequency: Сurrently configured PWM signal frequency in Hz. Read-only.
 */
typedef struct {
	TIM_HandleTypeDef *htim;
	float frequency;
} PWM_Timer;

/**
 * @brief PWM channel handle
 *
 * Fields:
 * - timer:     Associated timer
 * - name:      Timer channel (e.g., TIM_CHANNEL_1)
 * - dutyCycle: Сurrently configured duty cycle. Read-only.
 */
typedef struct {
	PWM_Timer *timer;
	uint32_t name;
	float dutyCycle;
} PWM_Channel;

/**
 * @brief Starts PWM signal generation
 *
 * Enables output compare on the specified channel.
 *
 * @param channel Pointer to PWM channel
 */
void PWM_Start(PWM_Channel *channel);

/**
 * @brief Stops PWM signal generation
 *
 * Disables output compare on the specified channel.
 *
 * @param channel Pointer to PWM channel
 */
void PWM_Stop(PWM_Channel *channel);

/**
 * @brief Sets the PWM frequency
 *
 * This function reconfigures the timer to generate PWM signals
 * with the specified frequency.
 *
 * @note After calling this function, you must reapply the desired
 *       duty cycle using PWM_SetDutyCycle().
 *
 * @param timer     Pointer to PWM timer
 * @param frequency Desired frequency in range [Timer Clock / 2^32, Timer Clock / 2] Hz
 */
void PWM_SetFrequency(PWM_Timer *timer, float frequency);

/**
 * @brief Sets the PWM duty cycle
 *
 * Updates the compare value for the specified channel to adjust
 * the duty cycle.
 *
 * @param channel    Pointer to PWM channel
 * @param dutyCycle  Duty cycle in range [0.0, 100.0]
 */
void PWM_SetDutyCycle(PWM_Channel *channel, float dutyCycle);

#endif //PWM_H
