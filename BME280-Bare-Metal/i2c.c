#include "i2c.h"
#include "stm32f103_hal.h"

// =================================================================
// LAYER 1: I2C LOW-LEVEL IMPLEMENTATION
// =================================================================

void i2c_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIO;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->CRL &= ~GPIO_CRL_MODE6_7_MASK;
    GPIOB->CRL |=  GPIO_CRL_I2C1_AF_OD;

    I2C1->CR1 |=  I2C1_CR1_SWRST;
    I2C1->CR1 &= ~I2C1_CR1_SWRST;

    I2C1->CR2  = I2C_CR2_FREQ;
    I2C1->CCR  = I2C_CCR;

    // Standard mode 100 kHz: TRISE = (1000 ns / 125 ns) + 1 = 9
    I2C1->TRISE = I2C_TRISE;

    I2C1->CR1 |= I2C1_CR1_PE;
}

I2C_Status i2c_start(void) {
    I2C1->CR1 |= I2C1_CR1_START;

    uint32_t timeout = 50000U;
    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
        if (--timeout == 0) return I2C_STAT_TIMEOUT;
    }
    return I2C_OK;
}

void i2c_stop(void) {
    I2C1->CR1 |= I2C1_CR1_STOP;
}

I2C_Status i2c_send_addr(uint8_t addr, uint8_t rw) {
    uint32_t timeout = I2C_TIMEOUT;

    I2C1->DR = (addr << 1) | rw;

    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
        if (--timeout == 0) return I2C_STAT_TIMEOUT;
    }

    /* Clear ADDR flag by reading SR1 then SR2 */
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    return I2C_OK;
}

void i2c_write_byte(uint8_t data) {
    while (!(I2C1->SR1 & I2C1_SR1_TXE));
    I2C1->DR = data;
    while (!(I2C1->SR1 & I2C1_SR1_BTF));
}

uint8_t i2c_read_byte_ack(void) {
    I2C1->CR1 |= I2C1_CR1_ACK;
    while (!(I2C1->SR1 & I2C1_SR1_RXNE));
    return (uint8_t)I2C1->DR;
}

uint8_t i2c_read_byte_nack(void) {
    I2C1->CR1 &= ~I2C1_CR1_ACK;
    i2c_stop();
    while (!(I2C1->SR1 & I2C1_SR1_RXNE));
    return (uint8_t)I2C1->DR;
}

void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t data) {
    i2c_start();
    i2c_send_addr(addr, 0);
    i2c_write_byte(reg);
    i2c_write_byte(data);
    i2c_stop();
}

uint8_t i2c_read_reg(uint8_t addr, uint8_t reg) {
    i2c_start();
    i2c_send_addr(addr, 0);
    i2c_write_byte(reg);

    i2c_start();
    i2c_send_addr(addr, 1);
    return i2c_read_byte_nack();
}
