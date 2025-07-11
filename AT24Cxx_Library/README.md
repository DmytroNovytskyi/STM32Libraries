# STM32 AT24Cxx EEPROM Library

## Version

### *1.1*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the AT24Cxx EEPROM library.

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
  - **I2C Speed Mode** → `Standard Mode` (up to 100 kHz, ) or `Fast Mode` (up to 400 kHz), depending on your EEPROM's capabilities and pull-up resistor configuration.
  - **Clock Speed** → Desired frequency (e.g., `100000` for 100 kHz, `400000` for 400 kHz).

> ⚠️ **External Pull-up Resistors are MANDATORY for I2C bus lines (SDA and SCL)!**
>
> - For 100 kHz (Standard Mode), typical values are 4.7 kΩ to 10 kΩ.
> - For 400 kHz (Fast Mode), typical values are 1.8 kΩ to 2.2 kΩ.
>
---

#### 3. **Address Pins (A0, A1, A2) and Write Protect Pin (WP)**

The AT24Cxx series EEPROMs use pins A0, A1, and A2 to form the lower bits of their 7-bit I2C device address. These pins **must be connected to a defined logic level (GND or Vcc)**.

- Connect A0, A1, A2 to `GND` if you are using the default device address (0x50).
- If connecting multiple EEPROMs to the same bus, connect these pins to different combinations of `GND`/`Vcc` to assign unique addresses.

The WP pin on the AT24Cxx EEPROM is a critical control input that prevents write operations to the entire memory array when asserted.

- To ENABLE writing (normal operation): Connect the `WP` pin to `GND`.
- To DISABLE writing (write protect): Connect the `WP` pin to `Vcc`.

>⚠️ If you are experiencing issues with writing data to the EEPROM (e.g., HAL_I2C_ERROR_AF during write, but reads work), ensure that the WP pin is correctly connected to GND.
---

## Example Usage

```c
AT24CXX_HandleTypeDef eeprom = {
  .hi2c = &hi2c1,
  .deviceAddress = (AT24CXX_BASE_ADDRESS | 0x04) << 1, //A2=1, A1=0, A0=0
  .memorySizeKbit = 256,
  .pageSize = 64
};
uint16_t address = 0x0010;
uint8_t[2] dataToWrite = { 0xAA, 0xBB };
uint8_t[2] readData;

if(AT24CXX_IsDeviceReady(&eeprom, 3, 100)){
    AT24CXX_Erase(&eeprom, 500);
}

AT24CXX_Write(&eeprom, address, &dataToWrite, 2, 100);
AT24CXX_Read(&eeprom, address, &readData, 2, 100);
```

>⚠️  After each write operation (including `AT24CXX_Write` and `AT24CXX_Erase`), the EEPROM performs an internal write cycle. A small `5 ms` delay is included to account for this. For critical applications, it's recommended to use `AT24CXX_IsDeviceReady` after writes to ensure the device is truly idle before the next operation.
