#include <stdint.h>

//RCC
#define RCC_BASE 0x40021000UL
#define RCC_APB1ENR (*((volatile uint32_t*)(RCC_BASE + 0x1C)))
#define RCC_APB2ENR (*((volatile uint32_t*)(RCC_BASE + 0x18)))

//GPIOC
#define GPIOC_BASE 0x40011000UL
#define GPIOC_CRH (*((volatile uint32_t*)(GPIOC_BASE + 0x04)))
#define GPIOC_ODR (*((volatile uint32_t*)(GPIOC_BASE + 0x0C)))

//TIME2
#define TIM2_BASE 0x40000000UL
#define TIM2_CR1  (*((volatile uint32_t*)(TIM2_BASE + 0x00)))
#define TIM2_SR   (*((volatile uint32_t*)(TIM2_BASE + 0x10)))
#define TIM2_PSC  (*((volatile uint32_t*)(TIM2_BASE + 0x28)))
#define TIM2_ARR  (*((volatile uint32_t*)(TIM2_BASE + 0x2C)))
#define TIM2_DIER (*((volatile uint32_t*)(TIM2_BASE + 0x0C)))
#define TIM2_EGR  (*((volatile uint32_t*)(TIM2_BASE + 0x14)))
//NVIC
#define NVIC_ISER0   (*((volatile uint32_t*)0xE000E100))

volatile uint32_t ms_tick = 0;

//ISR chạy mỗi 1ms
void TIM2_IRQHandler(void){
   TIM2_SR &= ~(1 << 0);
   ms_tick++;
}

void timer_init(void){
   //enable clock TIM2 
   RCC_APB1ENR |= (1 << 0);
   
   //PSC chia 8MHz xuống 1000Hz vì mỗi tick = 1ms
   TIM2_PSC = 79;
   //ARR đếm đến 999 -> interrupt mỗi 1000 ticks 1s
   //muốn 1s thì 1ms nên ARR = 1000 - 1
   TIM2_ARR = 99;
   // Force update event để load PSC/ARR vào shadow register
   TIM2_EGR |= (1 << 0);         // UG bit
   TIM2_SR  &= ~(1 << 0);     
   //enablt tim2 interrupt
   TIM2_DIER |= (1 << 0);
   //enablt TIM2 interrupt trong NVIC
   NVIC_ISER0 |= (1 << 28);
   //Bật timer
   TIM2_CR1 |= (1 << 0);
}

void delay_ms(uint32_t ms){
   uint32_t start = ms_tick;
   while((ms_tick - start) < ms);
}
int main(void){
   //enable clock GPIOC
   RCC_APB2ENR |= (1 << 4);
   
   //PC13
   GPIOC_CRH &= ~(0x0F << 20);
   GPIOC_CRH |= (0x02 << 20);

   //khởi tạo timer
   timer_init();
   
   //bật interrupt toàn cục
   __asm("CPSIE I");
    
   while(1){
      GPIOC_ODR ^= (1 << 13);
      delay_ms(500);
   }
}
