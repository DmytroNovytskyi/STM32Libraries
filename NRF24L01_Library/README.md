# STM32 nRF24L01+ Library

## Version

### *1.0*

## Author

### *Dmytro Novytskyi*

## Configuration Guide

This guide describes how to configure your STM32 project `.ioc` file to work properly with the nRF24L01+ library.

### Required Configuration in `.ioc` (STM32CubeIDE)

#### 1. **Enable Debugging**

Navigate to:

> *System Core* → *SYS*

Set:

- **Debug** → `Serial Wire`

---

#### 2. **Configure ADC**

You can use any spi (e.g., SPI1)

Navigate to:

> *Connectivity* → *SPIx*

Set:

- **Mode** → Mode → `Full-Duplex Master`
- **Configuration** → Parameter Settings → Clock Parameters → Prescaler(for Baund Rate) → `<prescaler>` (prescaler = SYSCLK / Baund Rate. Recommended baund rate value to be not higher than 10 MBits/s. Stable and tested values are  1 and 2 Mbits/s)

---

#### 3. **Clock Configuration**

Navigate to:

Clock Configuration

Make sure:

- System clock(SYSCLK) values are configured with **SPI Baund Rate** in mind.

---

## Example Usage

### **Wiring diagram**

| nRF24L01+ Pin | STM32F103C8T6 Pin | Description             |
|---------------|-------------------|-------------------------|
| VCC           | 3.3               | Power Supply            |
| GND           | GND               | Ground                  |
| CE            | A3                | Chip Enable             |
| CSN           | A4                | Chip Select Not         |
| SCK           | A5                | SPI Clock               |
| MOSI          | A7                | SPI Master Out Slave In |
| MISO          | A6                | SPI Master In Slave Out |
| IRQ           | Not Used          | Interrupt (optional)    |

### **Code**

#### **Transmitter**

```c
NRF24L01_Device device = {
    .CE_Port = GPIOA,
    .CE_Pin = GPIO_PIN_3,
    .CSN_Port = GPIOA,
    .CSN_Pin = GPIO_PIN_4,
    .hspi = &hspi1,
    .enableStatistics = true,
    .powerDownBetweenTransactions = true
};

NRF24L01_Config config = {
    .addressWidth = ADDR_WIDTH_3BYTES,
    .retransmitCount = RETR_TIMES_15,
    .retransmitDelay = RETR_DELAY_1000US,
    .channel = 1,
    .rfPower = RF_PWR_NEG6DBM,
    .dataRate = DATA_RATE_250KBPS,
    .rxPipes = {
      { 0, true, true, 0x112233, 0, true }
    },
    .txPipeAddress = 0x112233,
    .enableDynamicPayloadSizeFeature = true
};

uint8_t data[32] = { 0 };
for (int i = 0; i < 32; i++) {
    data[i] = i;
}

NRF24L01_Init(&device, &config);
NRF24L01_Transmit(&device, data, 32, 18);
```

#### **Receiver**

```c
NRF24L01_Device device = {
    .CE_Port = GPIOA,
    .CE_Pin = GPIO_PIN_3,
    .CSN_Port = GPIOA,
    .CSN_Pin = GPIO_PIN_4,
    .hspi = &hspi1,
    .enableStatistics = true,
    .powerDownBetweenTransactions = true
};

NRF24L01_Config config = {
    .addressWidth = ADDR_WIDTH_3BYTES,
    .retransmitCount = RETR_TIMES_15,
    .retransmitDelay = RETR_DELAY_1000US,
    .channel = 1,
    .rfPower = RF_PWR_NEG6DBM,
    .dataRate = DATA_RATE_250KBPS,
    .rxPipes = {
        { 1, false, false, 0x002233, 0, false },
        { 5, true, true, 0x11, 0, true }
    },
    .enableDynamicPayloadSizeFeature = true
};

uint8_t buffer[32] = { 0 };
for (int i = 0; i < 32; i++) {
    data[i] = i;
}

NRF24L01_Init(&device, &config);
NRF24L01_Receive(&device, 5, buffer, 1000);
```

>⚠️ To ensure proper communication, adjust:
>
> - **NRF24L01_DEVICES** - a cache size for device configurations that optimizes data handling. Without it, the system might not function correctly.
