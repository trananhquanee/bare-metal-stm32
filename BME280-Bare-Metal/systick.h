#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

// =================================================================
// PUBLIC API
// =================================================================

/**
 * @brief  Initialize SysTick to fire every 1 ms (based on PCLK1/8).
 */
void Systick_init(void);

/**
 * @brief  SysTick interrupt handler — must be linked to the IRQ vector.
 *         Increments msTicks every 1 ms.
 */
void SysTick_Handler(void);

/**
 * @brief  Blocking delay in milliseconds.
 */
void delay_ms(uint32_t ms);

/** Global millisecond counter — incremented by SysTick_Handler. */
extern volatile uint32_t msTicks;

#endif /* SYSTICK_H */
