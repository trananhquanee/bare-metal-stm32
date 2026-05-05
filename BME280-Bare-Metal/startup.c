#include <stdint.h>

extern uint32_t _stack_top;

void Reset_Handler(void);
void SysTick_Handler(void);
int main(void);

void Default_Handler(void) { while(1); }

__attribute__((section(".isr_vector")))
uint32_t vector_table[] = {
    (uint32_t)&_stack_top,
    (uint32_t)&Reset_Handler,   // Reset
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
    (uint32_t)&SysTick_Handler, // SysTick ← đây
};

void Reset_Handler(void) {
    main();
    while(1);
}
