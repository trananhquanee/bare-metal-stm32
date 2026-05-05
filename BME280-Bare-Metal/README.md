# BME280 Bare-metal Driver — STM32F103

Reads temperature, pressure, humidity from BME280 over I2C.
Outputs formatted data via UART1 at 9600 baud.

## Hardware
- **MCU**: STM32F103C8T6 @ 8MHz HSI
- **BME280**: I2C1 (PB6=SCL, PB7=SDA), I2C Address: 0x76
- **UART TX**: PA9 (Connect to USB-TTL RX)

## Build
```bash
make
```

## Flash
```bash
make flash   # Using st-flash
```
