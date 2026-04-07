#include <stdint.h>

extern uint32_t _stack_top;

void Reset_Handler(void);
void TIM2_IRQHandler(void);  // khai báo thêm

int main(void);

// Default handler cho interrupt chưa dùng
void Default_Handler(void) { while(1); }

__attribute__((section(".isr_vector")))
uint32_t vector_table[] = {
    (uint32_t)&_stack_top,
    (uint32_t)&Reset_Handler,  // Reset
    (uint32_t)&Default_Handler, // NMI
    (uint32_t)&Default_Handler, // HardFault
    (uint32_t)&Default_Handler, // MemManage
    (uint32_t)&Default_Handler, // BusFault
    (uint32_t)&Default_Handler, // UsageFault
    0, 0, 0, 0,                 // Reserved
    (uint32_t)&Default_Handler, // SVCall
    (uint32_t)&Default_Handler, // Debug
    0,                          // Reserved
    (uint32_t)&Default_Handler, // PendSV
    (uint32_t)&Default_Handler, // SysTick
    // External interrupts bắt đầu từ đây
    (uint32_t)&Default_Handler, // WWDG
    (uint32_t)&Default_Handler, // PVD
    (uint32_t)&Default_Handler, // TAMPER
    (uint32_t)&Default_Handler, // RTC
    (uint32_t)&Default_Handler, // FLASH
    (uint32_t)&Default_Handler, // RCC
    (uint32_t)&Default_Handler, // EXTI0
    (uint32_t)&Default_Handler, // EXTI1
    (uint32_t)&Default_Handler, // EXTI2
    (uint32_t)&Default_Handler, // EXTI3
    (uint32_t)&Default_Handler, // EXTI4
    (uint32_t)&Default_Handler, // DMA1_CH1
    (uint32_t)&Default_Handler, // DMA1_CH2
    (uint32_t)&Default_Handler, // DMA1_CH3
    (uint32_t)&Default_Handler, // DMA1_CH4
    (uint32_t)&Default_Handler, // DMA1_CH5
    (uint32_t)&Default_Handler, // DMA1_CH6
    (uint32_t)&Default_Handler, // DMA1_CH7
    (uint32_t)&Default_Handler, // ADC1_2
    (uint32_t)&Default_Handler, // USB_HP
    (uint32_t)&Default_Handler, // USB_LP
    (uint32_t)&Default_Handler, // CAN_RX1
    (uint32_t)&Default_Handler, // CAN_SCE
    (uint32_t)&Default_Handler, // EXTI9_5
    (uint32_t)&Default_Handler, // TIM1_BRK
    (uint32_t)&Default_Handler, // TIM1_UP
    (uint32_t)&Default_Handler, // TIM1_TRG
    (uint32_t)&Default_Handler, // TIM1_CC
    (uint32_t)&TIM2_IRQHandler, // TIM2 ← đây
};

void Reset_Handler(void) {
    main();
    while(1);
}
