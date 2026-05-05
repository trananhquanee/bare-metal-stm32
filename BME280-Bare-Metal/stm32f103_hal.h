#ifndef STM32F103_HAL_H
#define STM32F103_HAL_H

#include <stdint.h>

// =================================================================
// REGION 1: REGISTER TYPEDEFS & BASE ADDRESSES
// =================================================================

/* --- RCC --- */
typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_Typedef;
#define RCC_BASE  0x40021000UL
#define RCC       ((RCC_Typedef *)RCC_BASE)

/* --- GPIO --- */
typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_Typedef;
#define GPIOA_BASE  0x40010800UL
#define GPIOB_BASE  0x40010C00UL
#define GPIOA       ((GPIO_Typedef *)GPIOA_BASE)
#define GPIOB       ((GPIO_Typedef *)GPIOB_BASE)

/* --- I2C --- */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_Typedef;
#define I2C1_BASE  0x40005400UL
#define I2C1       ((I2C_Typedef *)I2C1_BASE)

/* --- USART --- */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_Typedef;
#define USART1_BASE  0x40013800UL
#define USART1       ((USART_Typedef *)USART1_BASE)

/* --- SysTick --- */
typedef struct {
    volatile uint32_t CSR;
    volatile uint32_t RVR;
    volatile uint32_t CVR;
    volatile uint32_t CALIB;
} SYSTICK_Typedef;
#define SYSTICK_BASE  0xE000E010UL
#define SYSTICK       ((SYSTICK_Typedef *)SYSTICK_BASE)

// =================================================================
// REGION 2: REGISTER BIT DEFINITIONS
// =================================================================

/* --- RCC APB2ENR --- */
#define RCC_APB2ENR_AFIO       (1U << 0)
#define RCC_APB2ENR_IOPAEN     (1U << 2)
#define RCC_APB2ENR_IOPBEN     (1U << 3)
#define RCC_APB2ENR_USART1EN   (1U << 14)

/* --- RCC APB1ENR --- */
#define RCC_APB1ENR_I2C1EN     (1U << 21)

/* --- GPIO --- */
#define GPIO_CRL_MODE6_7_MASK  (0xFFUL << 24)
#define GPIO_CRL_I2C1_AF_OD    (0xEEUL << 24)
#define GPIO_CRH_MODE9_MASK    (0x0FUL << 4)
#define GPIO_CRH_USART1_AF_PP  (0x0AUL << 4)

/* --- I2C SR1 / SR2 / CR1 --- */
#define I2C1_SR1_SB    (1U << 0)
#define I2C1_SR1_ADDR  (1U << 1)
#define I2C1_SR1_BTF   (1U << 2)
#define I2C1_SR1_RXNE  (1U << 6)
#define I2C1_SR1_TXE   (1U << 7)
#define I2C1_SR2_BUSY  (1U << 1)
#define I2C1_CR1_PE    (1U << 0)
#define I2C1_CR1_START (1U << 8)
#define I2C1_CR1_STOP  (1U << 9)
#define I2C1_CR1_ACK   (1U << 10)
#define I2C1_CR1_SWRST (1U << 15)

/* --- USART --- */
#define USART1_SR_TXE   (1U << 7)
#define USART1_SR_TC    (1U << 6)
#define USART1_CR1_UE   (1U << 13)
#define USART1_CR1_TE   (1U << 3)

/* --- SysTick CSR --- */
#define SYSTICK_INIT       (1U << 1)
#define SYSTICK_CLKSOURCE  (1U << 2)
#define CSR_ENABLE         (1U << 0)

// =================================================================
// REGION 3: SYSTEM CONSTANTS & CONFIGURATION
// =================================================================
#define PCLK1               8000000UL
#define I2C_CR2_FREQ        8
#define I2C_TIMEOUT         1000U
#define I2C_TRISE           9
#define I2C_CCR             40
#define USART1_BRR_VAL      833
#define SYSTICK_FREQ_HZ     1000
#define SYSTICK_TICK_COUNT  ((PCLK1 / 8) / SYSTICK_FREQ_HZ)

#endif /* STM32F103_HAL_H */
