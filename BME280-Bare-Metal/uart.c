#include "uart.h"
#include "stm32f103_hal.h"

// =================================================================
// LAYER 2: UART LOW-LEVEL IMPLEMENTATION
// =================================================================

void uart_init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~GPIO_CRH_MODE9_MASK;
    GPIOA->CRH |=  GPIO_CRH_USART1_AF_PP;

    USART1->BRR  = USART1_BRR_VAL;
    USART1->CR1 |= USART1_CR1_TE;
    USART1->CR1 |= USART1_CR1_UE;
}

void uart_send_char(char c) {
    while (!(USART1->SR & USART1_SR_TXE));
    USART1->DR = c;
}

void uart_send_string(const char *s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_int(int32_t n) {
    if (n == 0) {
        uart_send_char('0');
        return;
    }

    uint32_t num;
    if (n < 0) {
        uart_send_char('-');
        num = (uint32_t)(-(n + 1)) + 1;
    } else {
        num = (uint32_t)n;
    }

    char buf[10];
    uint8_t i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i > 0) uart_send_char(buf[--i]);
}

void uart_send_fixed(int32_t n, uint8_t decimals) {
    uint32_t num;

    if (n < 0) {
        uart_send_char('-');
        num = (uint32_t)(-(n + 1)) + 1;
    } else {
        num = (uint32_t)n;
    }

    int32_t divisor = 1;
    for (uint8_t i = 0; i < decimals; i++) divisor *= 10;

    int32_t integer_part  = (int32_t)(num / (uint32_t)divisor);
    int32_t fraction_part = (int32_t)(num % (uint32_t)divisor);

    uart_send_int(integer_part);

    if (decimals > 0) {
        uart_send_char('.');
        int32_t temp_div = divisor / 10;
        while (temp_div >= 1) {
            uint8_t digit = (uint8_t)(fraction_part / temp_div);
            uart_send_char(digit + '0');
            fraction_part %= temp_div;
            temp_div /= 10;
        }
    }
}

void uart_send_bme280_humidity(uint32_t h_raw) {
    uint32_t integer_part = h_raw >> 10;
    uint32_t remainder    = h_raw & 1023U;
    uint32_t decimal_part = (remainder * 100U) / 1024U;

    uart_send_int((int32_t)integer_part);
    uart_send_char('.');
    if (decimal_part < 10) uart_send_char('0');
    uart_send_int((int32_t)decimal_part);
    uart_send_char('%');
}
