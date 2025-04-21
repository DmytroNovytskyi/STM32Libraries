# STM32 PWM Library

## Version
### *1.0*

## Author
### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the PWM library.

### Required Configuration in `.ioc` (STM32CubeIDE)

#### 1. **Enable Debugging**

Navigate to:

> *System Core* → *SYS*

Set: 

- **Debug** → `Serial Wire`

---

#### 2. **Configure Timer for PWM**

You can use any general-purpose timer (e.g., TIM1, TIM2, etc.)

Navigate to:

> *Timers* → *TIMx*

Set:

- **Mode** → Channelx → `PWM Generation CHx`

In configuration leave **Prescaler** and **Counter Period (ARR)** default(auto-calculated by the library)

---

#### 3. **Clock Configuration**

Navigate to:

Clock Configuration


Make sure:

- The timer you are using (e.g., TIM1) has its input clock **enabled and active**.

> ⚠️ The timer clock frequency determines the **maximum and minimum** achievable PWM frequencies:
> - Higher timer clock → higher max PWM frequency, better resolution
> - Lower timer clock → lower min PWM frequency

---

## Example Usage

```c
PWM_Timer timer = { .htim = &htim1 };
PWM_Channel channel = { .timer = &timer, .name = TIM_CHANNEL_1 };

PWM_SetFrequency(&timer, 1000);      // 1kHz
PWM_SetDutyCycle(&channel, 50.0f);   // 50%
PWM_Start(&channel);
```
>⚠️ After calling `PWM_SetFrequency()`, the counter range (ARR) is changed.  
Because of this, you **must call `PWM_SetDutyCycle()` again** to restore the correct duty cycle.