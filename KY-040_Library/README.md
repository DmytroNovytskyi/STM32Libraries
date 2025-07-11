# STM32 KY-040 Rotary Encoder Library

## Version

### *1.0*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the KY-040 Rotary Encoder library.

### Required Configuration in `.ioc` (STM32CubeIDE)

#### 1. **Enable Debugging**

Navigate to:

> *System Core* → *SYS*

Set:

- **Mode**
  - **Debug** → `Serial Wire`

---

#### 2. **Configure Timer Peripheral (TIM) for Encoder Mode**

The KY-040 rotary encoder uses two outputs (`DT` and `CLK`) to generate pulses. These outputs must be connected to timer inputs configured in encoder mode.

Navigate to:

> *Timers* → *TIMx* (e.g., TIM1)

Set:

- **Mode**
  - **Combined Channels** → `Encoder Mode`
- **Parameter Settings**:
  - **Prescaler** → `0`
  - **Counter Period** → `65535`
  - **Encoder Mode** → `Encoder Mode TI1`
  - **Polarity** → `Rising Edge`
- **NVIC Settings**:
  - **TIMx global interrupt** → `Check`

>⚠️ Set the `Counter Period` to `65535` (the maximum value for a 16-bit unsigned integer, `0xFFFF`). This specific value ensures that when the counter underflows from `0`, it wraps around to `65535`, which correctly represents `-1` when cast to a signed 16-bit integer (`int16_t`). Using a different period will require custom logic to handle direction changes correctly.

---

#### 3. **Configure GPIO Pin for Button (SW)**

The button on the KY-040 is connected to a regular GPIO input.

Navigate to:

> *Pinout view*

Set:

- **Pxx (e.g., PA5)** → `GPIO_EXTIx (e.g., GPIO_EXTI5)`

Navigate to:

> *System Core* → *NVIC*

Set:

- **EXTI linex interrupts (e.g., EXTI line[9:5] interrupts)** → `Check`

Navigate to:

> *System Core* → *GPIO* → *Pxx (e.g., PA5)*

- **GPIO mode** → `External Interrupt Mode with Rising/Falling edge trigger detection`
- **GPIO Pull-up/Pull-down** → Select `Pull-up` if your button does not have an external pull-up resistor. The non-pressed state should be logic high (`1`), and the pressed state should be logic low (`0`).

---

## Example Usage

```c
KY_040_HandleTypeDef handle = {
  .htim = &htim3,
  .swPort = GPIOA,
  .swPin = GPIO_PIN_5
};

KY_040_Init(&handle);

int16_t encValue = KY_040_GetEncoderValue(&handle);
bool encPressed = KY_040_IsPressed(&handle);
```

>⚠️ The returned encoder value represents the change in steps since the last reading. The sign indicates direction (e.g., `-1` for one step `left`, `+1` for one step `right`). The value accumulates with each step and resets to zero after being read.
