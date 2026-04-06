#include <stdint.h>

#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x40010800UL
#define USART1_BASE 0x40013800UL

// RCC
#define RCC_APB2ENR (*((volatile uint32_t*)(RCC_BASE + 0x18)))

// GPIOA
#define GPIOA_CRH (*((volatile uint32_t*)(GPIOA_BASE + 0x04)))
#define GPIOA_ODR (*((volatile uint32_t*)(GPIOA_BASE + 0x0C)))

// USART1
#define USART1_SR  (*((volatile uint32_t*)(USART1_BASE + 0x00)))
#define USART1_DR  (*((volatile uint32_t*)(USART1_BASE + 0x04)))
#define USART1_BRR (*((volatile uint32_t*)(USART1_BASE + 0x08)))
#define USART1_CR1 (*((volatile uint32_t*)(USART1_BASE + 0x0C)))

// SR bits
#define TXE  7   // TX buffer trống
#define RXNE 5   // RX buffer có dữ liệu

void uart_send_char(char c) {
    while (!(USART1_SR & (1 << TXE)));
    USART1_DR = c;
}

void uart_send_string(const char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

char uart_receive_char(void) {
    while (!(USART1_SR & (1 << RXNE)));  // chờ có dữ liệu
    return (char)USART1_DR;              // đọc và trả về
}

int main(void) {
    // Enable clock
    RCC_APB2ENR |= (1 << 0) | (1 << 2) | (1 << 14);

    // PA9 TX — AF Push-Pull 2MHz
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xA << 4);

    // PA10 RX — Input Floating
    GPIOA_CRH &= ~(0xF << 8);
    GPIOA_CRH |=  (0x4 << 8);  // CNF=01 (floating), MODE=00

    // USART1 config
    USART1_BRR  = 833;
    USART1_CR1 |= (1 << 3);   // TE
    USART1_CR1 |= (1 << 2);   // RE — enable receiver
    USART1_CR1 |= (1 << 13);  // UE

    uart_send_string("=== UART Ready ===\r\n");
    uart_send_string("Gõ gì thì nhận lại cái đó:\r\n");

    while (1) {
        char c = uart_receive_char();  // chờ nhận ký tự
        uart_send_char(c);             // echo lại

        // Nếu nhận Enter thì xuống dòng
        if (c == '\r') {
            uart_send_char('\n');
        }
    }
}
