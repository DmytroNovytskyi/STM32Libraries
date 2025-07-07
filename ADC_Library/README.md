# STM32 ADC Library

## Version

### *1.0*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the ADC library.

### Required Configuration in `.ioc` (STM32CubeIDE)

#### 1. **Enable Debugging**

Navigate to:

> *System Core* → *SYS*

Set:

- **Debug** → `Serial Wire`

---

#### 2. **Configure ADC**

You can use any Analog-to-Digital Converter (e.g., ADC1, etc.)

Navigate to:

> *Analog* → *ADCx*

Set:

- **Mode** → Channelx → `Checked`

In configuration leave everthing default(configured by the library)

---

#### 3. **Clock Configuration**

Navigate to:

Clock Configuration

Make sure:

- The ADC you are using (e.g., ADC1) has its input clock **enabled and active**.

---

## Example Usage

```c
ADC_Calibrate(&hadc1); //Recommened to do during system initialization

uint16_t value = ADC_ReadValue(&hadc1, ADC_CHANNEL_0); //Read value from ADC1 IN0
```
