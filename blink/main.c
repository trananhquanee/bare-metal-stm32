#include <stdint.h>

// GHI LẠI: Địa chỉ lấy từ RM0008, mục "Memory Map"
#define RCC_BASE    0x40021000UL
#define GPIOC_BASE  0x40011000UL

// RCC_APB2ENR: register enable clock cho GPIO
// offset 0x18 từ RCC_BASE
#define RCC_APB2ENR  (*((volatile uint32_t*)(RCC_BASE + 0x18)))

// GPIOC registers
#define GPIOC_CRH    (*((volatile uint32_t*)(GPIOC_BASE + 0x04)))
#define GPIOC_ODR    (*((volatile uint32_t*)(GPIOC_BASE + 0x0C)))

void delay(volatile uint32_t count) {
    while (count--);
}

int main(void) {
    // Bước 1: Enable clock GPIOC — bit 4 của APB2ENR
    RCC_APB2ENR |= (1 << 4);

    // Bước 2: Config PC13 là output push-pull, 2MHz
    // PC13 nằm ở CRH, bits [23:20]
    GPIOC_CRH &= ~(0xF << 20);  // clear
    GPIOC_CRH |=  (0x2 << 20);  // MODE=10 (output 2MHz), CNF=00 (push-pull)

    // Bước 3: Toggle LED mãi mãi
    while (1) {
        GPIOC_ODR |=  (1 << 13);  // LED tắt (active low)
        delay(500000);
        GPIOC_ODR &= ~(1 << 13);  // LED sáng
        delay(500000);
    }
}
