# STM32 KY-023 Library

## Version

### *1.0*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the KY-023 library.

### Required Configuration in `.ioc` (STM32CubeIDE)

#### 1. **Enable Debugging**

Navigate to:

> *System Core* → *SYS*

Set:

- **Debug** → `Serial Wire`

---

#### 2. **Configure ADC**

You can use any adc (e.g., ADC1)

Navigate to:

> *Analog* → *ADCx*

Set three channels (X tilt, Y tilt, Switch):

- **Mode** → INx → `Check`
- **Configuration** → Parameter Settings → ADC_Regular_ConversionMode → `Enable`

Anything under ADC_Regular_ConversionMode can be left default(auto-configured by the library)

---

#### 3. **Clock Configuration**

Navigate to:

Clock Configuration

Make sure:

- The adc you are using (e.g., ADC1) has its input clock **enabled and active**.

---

## Example Usage

### **Wiring diagram**

| KY-023 Pin | STM32F103C8T6 Pin | Description     |
|------------|-------------------|-----------------|
| GND        | GND               | Ground          |
| +5V        | 3.3               | Power Supply    |
| VRx        | A0                | Joystick X-axis |
| VRy        | A1                | Joystick Y-axis |
| SW         | A2                | Joystick Switch |

### **Code**

```c
 KY_023_Device joystick = {
   .hadc = &hadc1,
   .channelX = ADC_CHANNEL_0,
   .channelY = ADC_CHANNEL_1,
   .channelSW = ADC_CHANNEL_2,
 };
 KY_023_State state = KY_023_ReadState(&joystick)
```

>⚠️ Each joystick may have its own drift and button sensitivity.
>To handle this, adjust:
>
> - **KY_023_DRIFT_DEADZONE** ignores small analog changes when the joystick is idle.
> - **KY_023_PRESSED_THRESHOLD** sets the analog value at which the button is considered pressed.
Tuning these ensures more consistent behavior across different joysticks.
