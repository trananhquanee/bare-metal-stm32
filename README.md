# bare-metal-stm32

Learning repository for bare-metal STM32 development in C without HAL.

This repository starts from basic peripheral control and gradually expands toward deeper embedded topics.

## Current examples

* GPIO blink
* Button input
* UART transmit / receive
* Timer interrupt

## Project structure

* `blink/` : LED blinking
* `button/` : GPIO input
* `uart/` : UART communication
* `timer/` : Timer interrupt

## Toolchain

* arm-none-eabi-gcc
* Makefile
* OpenOCD

## Board

* STM32F103 (Blue Pill)

## Learning focus

* Register-level programming
* Startup code
* Linker script
* Interrupt handling
* Peripheral initialization

## Planned topics

* PWM
* ADC
* SPI
* I2C
* DMA
* NVIC details
* Low-power modes
* RTOS concepts
