# STM32 SSD1306 OLED Driver Library

## Version

### *1.0*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the SSD1306 OLED display driver library.

### Required Configuration in `.ioc` (STM32CubeIDE)

#### 1. **Enable Debugging**

Navigate to:

> *System Core* → *SYS*

Set:

- **Debug** → `Serial Wire`

---

#### 2. **Configure I2C Peripheral**

You can use any available I2C peripheral (e.g., I2C1, I2C2).

Navigate to:

> *Connectivity* → *I2Cx* (e.g., I2C1)

Set:

- **Mode** → `I2C`
- **Parameter Settings**:
  - **I2C Speed Mode** → `Fast Mode`
  - **Clock Speed** →  `400000`

---

#### 3. **Display Address**

SSD1306 OLED displays typically have a fixed I2C slave address. Common addresses are `0x3C` (for displays with SA0 pin connected to GND) or `0x3D` (for displays with SA0 pin connected to VCC). Refer to your specific display module's datasheet.

When setting the `address` field in the `SSD1306_HandleTypeDef` structure, the 7-bit I2C slave address **must be left-shifted by 1 bit** to convert it to an 8-bit address format expected by the STM32 HAL I2C functions.

- For a 7-bit address of `0x3C`, set `handle->address = 0x3C << 1;` which results in `0x78`.
- For a 7-bit address of `0x3D`, set `handle->address = 0x3D << 1;` which results in `0x7A`.

The `SSD1306_Init` function expects this 8-bit (shifted) address in its `handle->address` parameter.

---

## Example Usage

For practical examples of how to use the functions in this library, please refer to the test files: `SSD1306_Tests.h` and `SSD1306_Tests.c`. These files contain various test routines demonstrating the use of drawing primitives, text, and display functionalities.

```c
SSD1306_HandleTypeDef handle = {
   .hi2c = &hi2c1,
   .address = 0x3C << 1,
   .height = 64,
   .width = 128,
   .mirrorVertically = false,
   .mirrorHorizontally = false,
   .invertColor = false
 };

SSD1306_TestAll(&handle);
```

>⚠️  Additionally, when including the base header file (SSD1306.h), you will also need to include the font files you intend to use and **comment out the `#define` directives for any fonts that are not being used** within `SSD1306.h` . This is done to create flexibility in font selection and to avoid including unnecessary font data if not all fonts are required.
