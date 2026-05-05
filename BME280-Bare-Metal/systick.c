#include "systick.h"
#include "stm32f103_hal.h"

// =================================================================
// LAYER 3: SYSTICK IMPLEMENTATION
// =================================================================

volatile uint32_t msTicks = 0;

void Systick_init(void) {
    SYSTICK->CSR = 0;
    SYSTICK->RVR = SYSTICK_TICK_COUNT - 1;
    SYSTICK->CVR = 0;

    /* Enable SysTick with external clock (PCLK1/8), enable interrupt */
    SYSTICK->CSR = (SYSTICK_INIT) | ((CSR_ENABLE) & ~SYSTICK_CLKSOURCE);
}

void SysTick_Handler(void) {
    msTicks++;
}

void delay_ms(uint32_t ms) {
    uint32_t startTicks = msTicks;
    while ((msTicks - startTicks) < ms);
}
