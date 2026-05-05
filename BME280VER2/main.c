/**
 * @file    main.c
 * @brief   Bare-metal BME280 driver on STM32F103 (Blue Pill)
 *          Reads temperature, pressure, humidity over I2C
 *          and transmits formatted data via UART1 at 9600 baud.
 *
 * @hardware STM32F103C8T6 @ 8MHz HSI
 *           BME280 on I2C1 (PB6=SCL, PB7=SDA), addr 0x76
 *           UART1 TX on PA9
 *
 * @author  Tran Anh Quan
 * @date    2026-05-05
 */
#include <stdint.h>

// =================================================================
// LAYER 0: BIT MAP REGISTER
// =================================================================

/* --- REGION 1: Hardware Abstraction --- */
//RCC
typedef struct{
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
}RCC_Typedef;
#define RCC_BASE    0x40021000UL
#define RCC ((RCC_Typedef*)RCC_BASE)

//GPIOA
typedef struct{
   volatile uint32_t CRL;
   volatile uint32_t CRH;
   volatile uint32_t IDR;
   volatile uint32_t ODR;
   volatile uint32_t BSRR;
   volatile uint32_t BRR;
   volatile uint32_t LCKR;
}GPIO_Typedef;
#define GPIOA_BASE 0x40010800UL
#define GPIOB_BASE  0x40010C00UL

#define GPIOA ((GPIO_Typedef*)GPIOA_BASE)
#define GPIOB ((GPIO_Typedef*)GPIOB_BASE)

//I2C
typedef struct{
   volatile uint32_t CR1;
   volatile uint32_t CR2;
   volatile uint32_t OAR1;
   volatile uint32_t OAR2;
   volatile uint32_t DR;
   volatile uint32_t SR1;
   volatile uint32_t SR2;
   volatile uint32_t CCR;
   volatile uint32_t TRISE;
}I2C_Typedef;

#define I2C1_BASE 0x40005400UL
#define I2C1 ((I2C_Typedef*)I2C1_BASE)

//USART
typedef struct{
   volatile uint32_t SR;
   volatile uint32_t DR;
   volatile uint32_t BRR;
   volatile uint32_t CR1;
   volatile uint32_t CR2;
   volatile uint32_t CR3;
   volatile uint32_t GTPR;
}USART_Typedef;
#define USART1_BASE 0x40013800UL
#define USART1 ((USART_Typedef*)USART1_BASE)

//Systick
typedef struct{
   volatile uint32_t CSR;
   volatile uint32_t RVR;
   volatile uint32_t CVR;
   volatile uint32_t CALIB;
}SYSTICK_Typedef;
#define SYSTICK_BASE 0xE000E010UL
#define SYSTICK ((SYSTICK_Typedef*)SYSTICK_BASE)

/* --- REGION 2: Register Bit Definitions --- */
#define RCC_APB2ENR_IOPBEN       (1U << 3)
#define RCC_APB2ENR_IOPAEN       (1U << 2)
#define RCC_APB2ENR_AFIO         (1U << 0)
#define RCC_APB2ENR_USART1EN     (1U << 14)

#define RCC_APB1ENR_I2C1EN       (1U << 21)

#define GPIO_CRL_MODE6_7_MASK    (0xFFUL << 24)
#define GPIO_CRL_I2C1_AF_OD      (0xEEUL << 24)

#define GPIO_CRH_MODE9_MASK      (0x0FUL << 4)
#define GPIO_CRH_USART1_AF_PP    (0x0AUL << 4)

#define I2C1_SR1_SB           (1U << 0)
#define I2C1_SR1_ADDR         (1U << 1)
#define I2C1_SR1_BTF          (1U << 2)
#define I2C1_SR1_RXNE         (1U << 6)
#define I2C1_SR1_TXE          (1U << 7)
#define I2C1_SR2_BUSY         (1U << 1)
#define I2C1_CR1_PE           (1U << 0)
#define I2C1_CR1_START        (1U << 8)
#define I2C1_CR1_STOP         (1U << 9)
#define I2C1_CR1_ACK          (1U << 10)
#define I2C1_CR1_SWRST        (1U << 15)

#define USART1_SR_TXE         (1U << 7)
#define USART1_SR_TC          (1U << 6)

#define USART1_CR1_UE         (1U << 13)
#define USART1_CR1_TE         (1U << 3)

#define SYSTICK_INIT          (1U << 1)
#define SYSTICK_CLKSOURCE     (1U << 2)
#define CSR_ENABLE            (1U << 0)

/* --- REGION 3: Constants & Configuration --- */

#define PCLK1              8000000UL
#define I2C_CR2_FREQ       8
#define I2C_TIMEOUT        1000U
#define I2C_TRISE          9
#define I2C_CCR            40
#define USART1_BRR_VAL     833
#define SYSTICK_FREQ_HZ    1000
#define SYSTICK_TICK_COUNT ((PCLK1 / 8) / SYSTICK_FREQ_HZ)

/* --- REGION 4: EXTERNAL DEVICE DRIVER — BME280 --- */
/* SECTION 1: Device Identity & Register Map */
#define BME280_I2C_ADDR      0x76
#define BME280_CHIP_ID_VALUE 0x60

/* Register Map */
#define BME280_REG_ID             0xD0
#define BME280_REG_CTRL_HUM       0xF2
#define BME280_REG_STATUS         0xF3
#define BME280_REG_CTRL_MEAS      0xF4
#define BME280_REG_CONFIG         0xF5
#define BME280_REG_CALIB_T1_BASE  0x88
#define BME280_REG_CALIB_H1       0xA1
#define BME280_REG_CALIB_H2_BASE  0xE1
#define BME280_REG_DATA_START     0xF7
/* SECTION 2: Bit Definitions & Configuration Masks */

/* Settings for CTRL_HUM (0xF2) */
#define BME280_OSRS_H            0x01

/* Settings for CTRL_MEAS (0xF4) */
#define BME280_OSRS_P        (0x01 << 2)
#define BME280_OSRS_T        (0x01 << 5)
#define BME280_MODE_NORMAL   0x03

/* Settings for CONFIG (0xF5) */
#define BME280_FILTER_OFF    0x00
#define BME280_FILTER     (0x02 << 2)

/* SECTION 3: Driver Data Structures */
typedef struct{
   //temperature
   uint16_t dig_T1;
   int16_t  dig_T2;
   int16_t  dig_T3;
   //pressure
   uint16_t dig_P1;
   int16_t  dig_P2;
   int16_t  dig_P3;
   int16_t  dig_P4;
   int16_t  dig_P5;
   int16_t  dig_P6;
   int16_t  dig_P7;
   int16_t  dig_P8;
   int16_t  dig_P9;
   //humidity
   uint8_t  dig_H1;
   int16_t  dig_H2;
   uint8_t  dig_H3;
   int16_t  dig_H4;
   int16_t  dig_H5;
   int8_t   dig_H6;
}BME280_Calib_t;
BME280_Calib_t calib;

typedef struct{
   uint32_t press;
   uint32_t temp;
   uint32_t hum;
}BME280_raw_t;
BME280_raw_t raw;

typedef enum {
    I2C_OK      = 0,
    I2C_ERROR   = 1,
    I2C_STAT_TIMEOUT = 2
} I2C_Status;

/* --- REGION 5: External Global Variables --- */

typedef int32_t  BME280_S32_t;
typedef uint32_t BME280_U32_t;
typedef int64_t  BME280_S64_t;

volatile uint32_t msTicks = 0;
// =========================================================
// LAYER 1: I2C LOW-LEVEL
// =========================================================

void i2c_init(void){
   RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIO;
   RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

   GPIOB->CRL   &= ~GPIO_CRL_MODE6_7_MASK;
   GPIOB->CRL   |= GPIO_CRL_I2C1_AF_OD;

   I2C1->CR1    |= I2C1_CR1_SWRST;
   I2C1->CR1    &= ~(I2C1_CR1_SWRST);

   I2C1->CR2    = I2C_CR2_FREQ;

   I2C1->CCR    = I2C_CCR;

   //(standard mode: 1000ns (100Khz) và Fast mode (400Khz) 300ns), TRISE_MAX/PCLK + 1
   // TRISE: 1000(ns)/(125ns) + 1 = 9 (125ns = 1/PCLK1 = 1/8Mhz)
   I2C1->TRISE = I2C_TRISE;

   I2C1->CR1 |= I2C1_CR1_PE;
}

static I2C_Status i2c_start(void) {
    I2C1->CR1 |= I2C1_CR1_START;

    uint32_t timeout = 50000U;
    while (!(I2C1->SR1 & I2C1_SR1_SB)) {
        if (--timeout == 0) {
            return I2C_STAT_TIMEOUT;
        }
    }

    return I2C_OK;
}static void i2c_stop(void){
   I2C1->CR1 |= I2C1_CR1_STOP;
}

I2C_Status i2c_send_addr(uint8_t addr, uint8_t rw) {
    uint32_t timeout = I2C_TIMEOUT;

    I2C1->DR = (addr << 1) | rw;

    while (!(I2C1->SR1 & I2C1_SR1_ADDR)) {
        if (--timeout == 0) return I2C_STAT_TIMEOUT;
    }

    (void)I2C1->SR1;
    (void)I2C1->SR2;
    return I2C_OK;
}

void i2c_write_byte(uint8_t data){
   while(!(I2C1->SR1 & I2C1_SR1_TXE));
   I2C1->DR = data;
   while(!(I2C1->SR1 & I2C1_SR1_BTF));
}

uint8_t i2c_read_byte_ack(void){
   I2C1->CR1 |= I2C1_CR1_ACK;
   while(!(I2C1->SR1 & I2C1_SR1_RXNE));
   return (uint8_t)I2C1->DR;
}

uint8_t i2c_read_byte_nack(void){
   I2C1->CR1 &= ~I2C1_CR1_ACK;

   i2c_stop();
   while(!(I2C1->SR1 & I2C1_SR1_RXNE));
   return (uint8_t)I2C1->DR;
}

void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t data){
   i2c_start();
   i2c_send_addr(addr, 0);
   i2c_write_byte(reg);
   i2c_write_byte(data);
   i2c_stop();
}

uint8_t i2c_read_reg(uint8_t addr, uint8_t reg){
   i2c_start();
   i2c_send_addr(addr, 0);
   i2c_write_byte(reg);

   i2c_start();
   i2c_send_addr(addr, 1);
   return (uint8_t)i2c_read_byte_nack();
}

// =====================================================
// LAYER 2: UART LOW-LEVEL
// =====================================================
void uart_init(void){
   RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
   GPIOA->CRH &= ~GPIO_CRH_MODE9_MASK;
   GPIOA->CRH |= GPIO_CRH_USART1_AF_PP;

   USART1->BRR = USART1_BRR_VAL;

   USART1->CR1 |= USART1_CR1_TE;
   USART1->CR1 |= USART1_CR1_UE;
}

void uart_send_char(char c){

   while(!(USART1->SR & USART1_SR_TXE));
   USART1->DR = c;
}

void uart_send_string(const char* s){
   while(*s)uart_send_char(*s++);
}

void uart_send_int(int32_t n){
   if(n == 0){
      uart_send_char('0');
      return;
   }
   uint32_t num;
   if(n < 0){
      uart_send_char('-');
      num = (uint32_t)-(n+1) + 1;
   }
   else{
      num = (uint32_t)n;
   }
   char buf[10];
   uint8_t i = 0;
   while(num > 0){
      buf[i++] = '0' + (num%10);
      num /= 10;
   }
   while(i > 0){
      uart_send_char(buf[--i]);
   }
}
/**
 * @brief Send signed integer with fixed decimal point.
 *        e.g. uart_send_fixed(2945, 2) → "29.45"
 */
void uart_send_fixed(int32_t n, uint8_t decimals){
   uint32_t num = 0;

   if(n < 0){
      uart_send_char('-');
      num = (uint32_t)-(n + 1) + 1;
   }
   else {
      num = (uint32_t)n;
   }

   int32_t divisor = 1;
   for(uint8_t i = 0; i < decimals; i++)divisor *= 10;

   int integer_part  = num/divisor;
   int fraction_part = num%divisor;

   uart_send_int(integer_part);

      if(decimals > 0){
      uart_send_char('.');
      int32_t temp_div = divisor / 10;

      while (temp_div >= 1) {
         uint8_t digit = fraction_part / temp_div;
         uart_send_char(digit + '0');
         fraction_part %= temp_div;
         temp_div /= 10;
      }
   }
}
/**
 * @brief Send BME280 humidity from Q22.10 raw format.
 *        e.g. h_raw=70656 → "69.00%"
 */
void uart_send_bme280_humidity(uint32_t h_raw) {
    uint32_t integer_part = h_raw >> 10;
    uint32_t remainder = h_raw & 1023;
    uint32_t decimal_part = (remainder * 100) / 1024;

    uart_send_int(integer_part);
    uart_send_char('.');
    if (decimal_part < 10) uart_send_char('0');
    uart_send_int(decimal_part);
    uart_send_char('%');
}
// ========================================================
// LAYER 3: SYSTICK
// ========================================================
void Systick_init(void){
   SYSTICK->CSR = 0;
   SYSTICK->RVR = SYSTICK_TICK_COUNT - 1;
   SYSTICK->CVR = 0;
   SYSTICK->CSR = (SYSTICK_INIT) | ((CSR_ENABLE) & ~ SYSTICK_CLKSOURCE);
}

void SysTick_Handler(void){
   msTicks ++;
}

void delay_ms(uint32_t ms){
   uint32_t startTicks= msTicks;
   while((msTicks - startTicks) < ms);
}
// ==================================================
//LAYER 4: BME280
// ==================================================
uint8_t bme280_chip_verify(void){
   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 0);
   i2c_write_byte(BME280_REG_ID);

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 1);
   uint8_t chip_id = i2c_read_byte_nack();

   return (chip_id == BME280_CHIP_ID_VALUE)? 1 : 0;
}

void bme280_read_calib(BME280_Calib_t *Cal){
   uint8_t buf1[24];
   uint8_t buf2[7];

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 0);
   i2c_write_byte(0x88);

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 1);
   for(int i =0; i < 23; i++){
      buf1[i] = i2c_read_byte_ack();
   }
   buf1[23] = i2c_read_byte_nack();

   Cal->dig_T1 = (buf1[1] << 8) | buf1[0];
   Cal->dig_T2 = (int16_t)((buf1[3] << 8) | buf1[2]);
   Cal->dig_T3 = (int16_t)((buf1[5] << 8) | buf1[4]);

   Cal->dig_P1 = (buf1[7] << 8) | buf1[6];
   Cal->dig_P2 = (int16_t)((buf1[9] << 8) | buf1[8]);
   Cal->dig_P3 = (int16_t)((buf1[11] << 8) | buf1[10]);
   Cal->dig_P4 = (int16_t)((buf1[13] << 8) | buf1[12]);
   Cal->dig_P5 = (int16_t)((buf1[15] << 8) | buf1[14]);
   Cal->dig_P6 = (int16_t)((buf1[17] << 8) | buf1[16]);
   Cal->dig_P7 = (int16_t)((buf1[19] << 8) | buf1[18]);
   Cal->dig_P8 = (int16_t)((buf1[21] << 8) | buf1[20]);
   Cal->dig_P9 = (int16_t)((buf1[23] << 8) | buf1[22]);

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 0);
   i2c_write_byte(0xA1);

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 1);
   Cal->dig_H1 = i2c_read_byte_nack();

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 0);
   i2c_write_byte(0xE1);

   i2c_start();
   i2c_send_addr(BME280_I2C_ADDR, 1);
   for(int j = 0; j < 6; j++){
      buf2[j]= i2c_read_byte_ack();
   }
   buf2[6] = i2c_read_byte_nack();

   Cal->dig_H2 = (int16_t)((buf2[1] << 8) | buf2[0]);
   Cal->dig_H3 = buf2[2];
   Cal->dig_H4 = (int16_t)((buf2[3] << 4) | (buf2[4] & 0x0F));
   Cal->dig_H5 = (int16_t)((buf2[5] << 4) | (buf2[4] >> 4));
   Cal->dig_H6 = (int8_t)(buf2[6]);
}

void bme280_config(void){
   i2c_write_reg(BME280_I2C_ADDR, BME280_REG_CTRL_HUM , BME280_OSRS_H);
   i2c_write_reg(BME280_I2C_ADDR, BME280_REG_CONFIG   , BME280_FILTER);
   i2c_write_reg(BME280_I2C_ADDR, BME280_REG_CTRL_MEAS, BME280_OSRS_T | BME280_OSRS_P | BME280_MODE_NORMAL);
}
/**
 * @brief Read all 8 raw ADC bytes from 0xF7 in one burst read.
 *        Blocks until sensor measurement is complete (status bit 3).
 */
void bme280_read_raw(void) {
    uint8_t buf[8];

    while (i2c_read_reg(BME280_I2C_ADDR, BME280_REG_STATUS) & (1 << 3));

    if (i2c_start() != I2C_OK) {
        uart_send_string("[ERROR] I2C Start Timeout!\r\n");
        return;
    }
    i2c_send_addr(BME280_I2C_ADDR, 0);
    i2c_write_byte(0xF7);

    if (i2c_start() != I2C_OK) {
        uart_send_string("[ERROR] I2C Restart Timeout!\r\n");
        return;
    }
    i2c_send_addr(BME280_I2C_ADDR, 1);

    for (uint8_t t = 0; t < 7; t++) {
        buf[t] = i2c_read_byte_ack();
    }
    buf[7] = i2c_read_byte_nack();

    raw.press = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | ((uint32_t)buf[2] >> 4);
    raw.temp  = ((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4) | ((uint32_t)buf[5] >> 4);
    raw.hum   = ((uint32_t)buf[6] << 8)  | ((uint32_t)buf[7]);
}
//// Refer to section 4.2.3 of the BME280 datasheet for the compensation formula.

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC.
// t_fine carries fine temperature as global value
BME280_S32_t t_fine;
BME280_S32_t BME280_compensate_T_int32(BME280_Calib_t *p_calib, BME280_S32_t adc_T)
{
    BME280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BME280_S32_t)p_calib->dig_T1 << 1))) * ((BME280_S32_t)p_calib->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BME280_S32_t)p_calib->dig_T1)) * ((adc_T >> 4) - ((BME280_S32_t)p_calib->dig_T1)))) >> 12) * ((BME280_S32_t)p_calib->dig_T3) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of "24674867" represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BME280_U32_t BME280_compensate_P_int64(BME280_Calib_t *p_calib, BME280_S32_t adc_P)
{
    BME280_S64_t var1, var2, p;
    var1 = ((BME280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BME280_S64_t)p_calib->dig_P6;
    var2 = var2 + ((var1 * (BME280_S64_t)p_calib->dig_P5) << 17);
    var2 = var2 + (((BME280_S64_t)p_calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (BME280_S64_t)p_calib->dig_P3) >> 8) + ((var1 * (BME280_S64_t)p_calib->dig_P2) << 12);
    var1 = (((((BME280_S64_t)1) << 47) + var1)) * ((BME280_S64_t)p_calib->dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((BME280_S64_t)p_calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((BME280_S64_t)p_calib->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)p_calib->dig_P7) << 4);
    return (BME280_U32_t)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of "47445" represents 47445/1024 = 46.333 %RH
BME280_U32_t bme280_compensate_H_int32(BME280_Calib_t *p_calib, BME280_S32_t adc_H)
{
    BME280_S32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)p_calib->dig_H4) << 20) - (((BME280_S32_t)p_calib->dig_H5) * v_x1_u32r)) + ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)p_calib->dig_H6)) >> 10) * (((v_x1_u32r * ((BME280_S32_t)p_calib->dig_H3)) >> 11) +
                ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) * ((BME280_S32_t)p_calib->dig_H2) +
                8192) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)p_calib->dig_H1)) >> 4));

    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (BME280_U32_t)(v_x1_u32r >> 12);
}
// ==============================================================
// LAYER 5: MAIN
// ==============================================================
int main(void) {
    Systick_init();
    uart_init();
    i2c_init();

    uart_send_string("--- BME280 SYSTEM STARTING ---\r\n");

    if(!bme280_chip_verify()){
      uart_send_string("[ERROR] BME280 Sensor not found! System Halted.\r\n");
      while(1);
   }
   uart_send_string("[OK] BME280 Founded.\r\n");

   bme280_read_calib(&calib);
   uart_send_string("[OK] Calibration data loaded.\r\n");

   bme280_config();
   uart_send_string("[OK] Sensor configured. Entering Loop...\r\n");

   while(1){
        bme280_read_raw();

        int32_t  temp_final = BME280_compensate_T_int32(&calib, raw.temp);
        uint32_t press_final = BME280_compensate_P_int64(&calib, raw.press);
        uint32_t hum_final = bme280_compensate_H_int32(&calib, raw.hum);

        uart_send_string("T: ");
        uart_send_fixed(temp_final, 2);
        uart_send_string(" C | ");

        uart_send_string("P: ");
        uart_send_fixed(press_final / 256, 2);
        uart_send_string(" hPa | ");

        uart_send_string("H: ");
        uart_send_bme280_humidity(hum_final);
        uart_send_string("\r\n");

        delay_ms(1000);
   }
}
