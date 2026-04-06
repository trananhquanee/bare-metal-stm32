#include <stdint.h>

#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define GPIOC_BASE 0x40011000UL

#define RCC_APB2ENR (*((volatile uint32_t*)(RCC_BASE + 0x18)))

#define GPIOA_CRL (*((volatile uint32_t*)(GPIOA_BASE + 0x00)))
#define GPIOA_ODR (*((volatile uint32_t*)(GPIOA_BASE + 0x0C)))
#define GPIOA_IDR (*((volatile uint32_t*)(GPIOA_BASE +0X08)))

#define GPIOC_CRH (*((volatile uint32_t*)(GPIOC_BASE + 0X04)))
#define GPIOC_ODR (*((volatile uint32_t*)(GPIOC_BASE + 0X0C)))

void delay(volatile uint32_t count){
    while(count --);
}
int main(void){
    RCC_APB2ENR |= (1 << 2) | (1 << 4);

    GPIOA_CRL &= ~(0x0F << 0);
    GPIOA_CRL |=  (0x08 << 0);

    GPIOC_CRH &= ~(0X0F << 20);
    GPIOC_CRH |= (0X02 << 20);

    GPIOA_ODR |= (1 << 0);

    while(1){
        if(GPIOA_IDR & (1 << 0)){
            GPIOC_ODR |= (1 << 13);
        }else{
            GPIOC_ODR &= ~(1 << 13);
        }
    }
}
