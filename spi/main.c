#include <stdint.h>

// ============================================================
// LAYER 0: REGISTER DEFINITIONS
// ============================================================

// RCC
#define RCC_BASE    0x40021000UL
#define RCC_APB2ENR (*((volatile uint32_t*)(RCC_BASE + 0x18)))

// GPIOA
#define GPIOA_BASE  0x40010800UL
#define GPIOA_CRL   (*((volatile uint32_t*)(GPIOA_BASE + 0x00)))
#define GPIOA_CRH   (*((volatile uint32_t*)(GPIOA_BASE + 0x04)))
#define GPIOA_ODR   (*((volatile uint32_t*)(GPIOA_BASE + 0x0C)))

// SPI1
#define SPI1_BASE   0x40013000UL
#define SPI1_CR1    (*((volatile uint32_t*)(SPI1_BASE + 0x00)))
#define SPI1_CR2    (*((volatile uint32_t*)(SPI1_BASE + 0x04)))
#define SPI1_SR     (*((volatile uint32_t*)(SPI1_BASE + 0x08)))
#define SPI1_DR     (*((volatile uint32_t*)(SPI1_BASE + 0x0C)))

// SR bits
#define TXE   1   // TX buffer empty
#define RXNE  0   // RX buffer not empty
#define BSY   7   // SPI busy

// CS pin — PA4 (GPIO thường)
#define CS_LOW()   GPIOA_ODR &= ~(1 << 4)
#define CS_HIGH()  GPIOA_ODR |=  (1 << 4)

// W25Q128 instructions
#define W25_READ_ID      0x90
#define W25_READ_DATA    0x03
#define W25_WRITE_ENABLE 0x06
#define W25_PAGE_PROGRAM 0x02

// UART1 (debug)
#define USART1_BASE 0x40013800UL
#define USART1_SR   (*((volatile uint32_t*)(USART1_BASE + 0x00)))
#define USART1_DR   (*((volatile uint32_t*)(USART1_BASE + 0x04)))
#define USART1_BRR  (*((volatile uint32_t*)(USART1_BASE + 0x08)))
#define USART1_CR1  (*((volatile uint32_t*)(USART1_BASE + 0x0C)))

// ============================================================
// LAYER 1: SPI LOW-LEVEL
// ============================================================

void spi_init(void) {
    // Enable clock: GPIOA + SPI1 + AFIO
    RCC_APB2ENR |= (1 << 2) | (1 << 12) | (1 << 0);

    // PA4 (CS)  → Output Push-Pull 50MHz
    GPIOA_CRL &= ~(0xF << 16);
    GPIOA_CRL |=  (0x3 << 16);  // MODE=11, CNF=00

    // PA5 (SCK) → AF Push-Pull 50MHz
    GPIOA_CRL &= ~(0xF << 20);
    GPIOA_CRL |=  (0xB << 20);  // MODE=11, CNF=10

    // PA6 (MISO) → Input Float
    GPIOA_CRL &= ~(0xF << 24);
    GPIOA_CRL |=  (0x4 << 24);  // MODE=00, CNF=01

    // PA7 (MOSI) → AF Push-Pull 50MHz
    GPIOA_CRL &= ~(0xF << 28);
    GPIOA_CRL |=  (0xB << 28);  // MODE=11, CNF=10

    // CS idle HIGH
    CS_HIGH();

    // SPI1 CR1 config:
    // CPOL=0, CPHA=0 (Mode 0)
    // BR=010 → fPCLK/8 = 1MHz (an toàn cho lần đầu)
    // MSTR=1 (master mode)
    // DFF=0 (8-bit data)
    // SSM=1, SSI=1 (software CS management)
    SPI1_CR1 = (0 << 1)  |  // CPHA=0
               (0 << 1)  |  // CPOL=0
               (1 << 2)  |  // MSTR=1
               (2 << 3)  |  // BR=010 → /8
               (1 << 8)  |  // SSI=1
               (1 << 9);    // SSM=1

    // Enable SPI
    SPI1_CR1 |= (1 << 6);   // SPE=1
}

uint8_t spi_transfer(uint8_t data) {
    while (!(SPI1_SR & (1 << TXE)));   // chờ TX trống
    SPI1_DR = data;                     // gửi
    while (!(SPI1_SR & (1 << RXNE)));  // chờ RX có data
    return (uint8_t)SPI1_DR;           // đọc và trả về
}

// ============================================================
// LAYER 2: W25Q128 DRIVER
// ============================================================

void w25_write_enable(void) {
    CS_LOW();
    spi_transfer(W25_WRITE_ENABLE);
    CS_HIGH();
}

uint16_t w25_read_id(void) {
    CS_LOW();
    spi_transfer(W25_READ_ID);
    spi_transfer(0x00);  // dummy address byte 1
    spi_transfer(0x00);  // dummy address byte 2
    spi_transfer(0x00);  // dummy address byte 3
    uint8_t mfr = spi_transfer(0xFF);   // manufacturer ID
    uint8_t dev = spi_transfer(0xFF);   // device ID
    CS_HIGH();
    return (uint16_t)(mfr << 8) | dev;
}

void w25_read_data(uint32_t addr, uint8_t* buf, uint32_t len) {
    CS_LOW();
    spi_transfer(W25_READ_DATA);
    spi_transfer((addr >> 16) & 0xFF);  // address byte 1
    spi_transfer((addr >> 8)  & 0xFF);  // address byte 2
    spi_transfer((addr)       & 0xFF);  // address byte 3
    for (uint32_t i = 0; i < len; i++) {
        buf[i] = spi_transfer(0xFF);    // clock out data
    }
    CS_HIGH();
}

// ============================================================
// LAYER 3: UART DEBUG
// ============================================================

void uart_init(void) {
    RCC_APB2ENR |= (1 << 14) | (1 << 2);
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xA << 4);
    USART1_BRR  = 833;
    USART1_CR1 |= (1 << 3);
    USART1_CR1 |= (1 << 13);
}

void uart_send_char(char c) {
    while (!(USART1_SR & (1 << 7)));
    USART1_DR = c;
}

void uart_send_string(const char* s) {
    while (*s) uart_send_char(*s++);
}

void uart_send_hex(uint8_t n) {
    const char hex[] = "0123456789ABCDEF";
    uart_send_string("0x");

    uart_send_char(hex[n >> 4]);
    uart_send_char(hex[n & 0xF]);
}

// ============================================================
// MAIN
// ============================================================

int main(void) {
    uart_init();
    spi_init();

    uart_send_string("SPI Init OK\r\n");

    // Đọc ID — verify chip
    uint16_t id = w25_read_id();
    uart_send_string("Manufacturer ID: ");
    uart_send_hex((id >> 8) & 0xFF);
    uart_send_string("\r\n");
    uart_send_string("Device ID: ");
    uart_send_hex(id & 0xFF);
    uart_send_string("\r\n");

    // Kết quả đúng:
    // Manufacturer ID: 0xEF
    // Device ID: 0x17

    while (1);
}
