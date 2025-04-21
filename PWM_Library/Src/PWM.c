/**
 * @brief Implementation of PWM library for STM32 using HAL
 *
 * Author: Dmytro Novytskyi
 * Version: 1.0
 */

#include "PWM.h"

static uint32_t PWM_GetTimerСlock(TIM_TypeDef *timer) {
	uint32_t pclk, timerClock;
	if (timer == TIM1) {
		pclk = HAL_RCC_GetPCLK2Freq();
		if ((RCC->CFGR & RCC_CFGR_PPRE2) != RCC_CFGR_PPRE2_DIV1) {
			timerClock = pclk * 2;
		} else {
			timerClock = pclk;
		}
	} else {
		pclk = HAL_RCC_GetPCLK1Freq();
		if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1) {
			timerClock = pclk * 2;
		} else {
			timerClock = pclk;
		}
	}
	return timerClock;
}

void PWM_Start(PWM_Channel *channel) {
	HAL_TIM_PWM_Start(channel->timer->htim, channel->name);
}

void PWM_Stop(PWM_Channel *channel) {
	HAL_TIM_PWM_Stop(channel->timer->htim, channel->name);
}

void PWM_SetFrequency(PWM_Timer *timer, float frequency) {
	TIM_TypeDef *timerInstance = timer->htim->Instance;
	HAL_TIM_Base_Stop(timer->htim); //Stop timer

	//Adjust prescaler and auto-reload register values
	uint32_t timerСlock = PWM_GetTimerСlock(timerInstance);
	uint32_t prescaler = 0;
	uint32_t arr = 0;
	for (prescaler = 0; prescaler <= 0xFFFF; prescaler++) {
		arr = (timerСlock / (frequency * (prescaler + 1))) - 1;
		if (arr <= 0xFFFF) {
			break;
		}
	}

	//Build configuration
	timerInstance->PSC = prescaler;
	timerInstance->ARR = arr;
	timerInstance->EGR = TIM_EGR_UG; //Update registers

	HAL_TIM_Base_Start(timer->htim); //Start timer
	timer->frequency = frequency;
}

void PWM_SetDutyCycle(PWM_Channel *channel, float dutyCycle) {
	TIM_TypeDef *timerInstance = channel->timer->htim->Instance;
	uint32_t arr = timerInstance->ARR;
	uint32_t ccr = (uint32_t) round(((dutyCycle / 100.0f) * (arr + 1)));

	//Sets CCR register value
	switch (channel->name) {
	case TIM_CHANNEL_1:
		timerInstance->CCR1 = ccr;
		break;
	case TIM_CHANNEL_2:
		timerInstance->CCR2 = ccr;
		break;
	case TIM_CHANNEL_3:
		timerInstance->CCR3 = ccr;
		break;
	case TIM_CHANNEL_4:
		timerInstance->CCR4 = ccr;
	}
	channel->dutyCycle = dutyCycle;
}
