#include <stdint.h>

//=====================================
// LAYER 1: REGISTER DEFINITIONS
//=====================================

//RCC
#define RCC_BASE     0x40021000UL
#define RCC_APB2ENR  (*((volatile uint32_t*)(RCC_BASE + 0x18)))
#define RCC_APB1ENR  (*((volatile uint32_t*)(RCC_BASE + 0X1C)))

//GPIOB
#define GPIOB_BASE   0x40010C00UL
#define GPIOB_ODR    (*((volatile uint32_t*)(GPIOB_BASE + 0x0C)))
#define GPIOB_CRL    (*((volatile uint32_t*)(GPIOB_BASE + 0x00)))

//GPIOA
#define GPIOA_BASE   0x40010800UL
#define GPIOA_CRH    (*((volatile uint32_t*)(GPIOA_BASE + 0x04)))
#define GPIOA_ODR    (*((volatile uint32_t*)(GPIOA_BASE + 0x0C)))

//I2C
#define I2C1_BASE     0x40005400UL
#define I2C1_CR1      (*((volatile uint32_t*)(I2C1_BASE + 0x00)))
#define I2C1_CR2      (*((volatile uint32_t*)(I2C1_BASE + 0X04)))
#define I2C1_DR       (*((volatile uint32_t*)(I2C1_BASE + 0x10)))
#define I2C1_SR1      (*((volatile uint32_t*)(I2C1_BASE + 0x14)))
#define I2C1_SR2      (*((volatile uint32_t*)(I2C1_BASE + 0x18)))
#define I2C1_CCR      (*((volatile uint32_t*)(I2C1_BASE + 0x1C)))
#define I2C1_TRISE    (*((volatile uint32_t*)(I2C1_BASE + 0x20)))

//SR1
#define SB           0   // Start Bit generated
#define ADDR         1   // Address sent
#define TXE          7   // TX buffer empty
#define RXNE         6   // RX buffer not empty
#define BTF          2   // Byte Transfer Finished

//DS3231
#define DS3231_ADDR  0x68

//UART1
#define USART1_BASE  0x40013800UL
#define USART1_SR    (*((volatile uint32_t*)(USART1_BASE + 0x00)))
#define USART1_DR    (*((volatile uint32_t*)(USART1_BASE + 0x04)))
#define USART1_BRR   (*((volatile uint32_t*)(USART1_BASE + 0x08)))
#define USART1_CR1   (*((volatile uint32_t*)(USART1_BASE + 0x0C)))

//=========================================
//LAYER 2: I2C LOW-LEVEL
//=========================================

void i2c_init(void){
   //BẬT RCC CHO I2C1, GPIOB, AFIO
   RCC_APB2ENR |= (1 << 3) | (1 << 0);
   RCC_APB1ENR |= (1 << 21);

   //PB6(SCL), PB7(SDA) Alternate Function Open-Drain
   GPIOB_CRL &= ~(0xFF << 24);
   GPIOB_CRL |= (0XFF << 24);

   //RESET I2C
   I2C1_CR1 |= (1 << 15);
   I2C1_CR1 &= ~(1 << 15);
   
   // CR2: peripheral clock = 8MHz
   I2C1_CR2 = 8;

   // CCR: standard mode 100kHz
   // T_high = T_low = 5us → CCR = 5us / (1/8MHz) = 40
   I2C1_CCR = 40;

   // TRISE: max rise time = 1000ns → TRISE = 1000ns/(1/8MHz) + 1 = 9
   I2C1_TRISE = 9;

   // Enable I2C1
   I2C1_CR1 |= (1 << 0);
}
void i2c_start(void){
   I2C1_CR1 |= (1 << 8);
   //chờ cờ SB lên 1
   while(!(I2C1_SR1 & (1 << SB)));
}

void i2c_stop(void){
   I2C1_CR1 |= (1 << 9);
}

void i2c_send_addr(uint8_t addr, uint8_t rw){
   I2C1_DR = (addr << 1) | rw; 
   while(!(I2C1_SR1 & ( 1 << ADDR)));
   (void)I2C1_SR1;       // clear ADDR flag
   (void)I2C1_SR2;       // bằng cách đọc SR1 + SR2
}
void i2c_write_byte(uint8_t data) {
    while (!(I2C1_SR1 & (1 << TXE)));        // chờ TXE=1
    I2C1_DR = data;
    while (!(I2C1_SR1 & (1 << BTF)));        // chờ BTF=1
}
uint8_t i2c_read_byte_ack(void) {
    I2C1_CR1 |= (1 << 10);                   // ACK=1
    while (!(I2C1_SR1 & (1 << RXNE)));
    return (uint8_t)I2C1_DR;
}

uint8_t i2c_read_byte_nack(void) {
    I2C1_CR1 &= ~(1 << 10);                  // ACK=0 → báo slave đây là byte cuối
    i2c_stop();
    while (!(I2C1_SR1 & (1 << RXNE)));
    return (uint8_t)I2C1_DR;
}
// ============================================================
// LAYER 2: DS3231 DRIVER
// ============================================================

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
} DS3231_Time;

uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

void ds3231_set_time(uint8_t h, uint8_t m, uint8_t s) {
    i2c_start();
    i2c_send_addr(DS3231_ADDR, 0);       // write mode
    i2c_write_byte(0x00);                // trỏ đến register 0x00
    i2c_write_byte(dec_to_bcd(s));       // seconds
    i2c_write_byte(dec_to_bcd(m));       // minutes
    i2c_write_byte(dec_to_bcd(h));       // hours
    i2c_stop();
}

DS3231_Time ds3231_get_time(void) {
    DS3231_Time t;

    // Ghi địa chỉ register muốn đọc
    i2c_start();
    i2c_send_addr(DS3231_ADDR, 0);       // write mode
    i2c_write_byte(0x00);               // trỏ đến register 0x00

    // Đọc 3 bytes
    i2c_start();                         // repeated start
    i2c_send_addr(DS3231_ADDR, 1);       // read mode
    t.seconds = bcd_to_dec(i2c_read_byte_ack() & 0x7F);
    t.minutes = bcd_to_dec(i2c_read_byte_ack() & 0x7F);
    t.hours   = bcd_to_dec(i2c_read_byte_nack() & 0x3F);

    return t;
}
//====================================================
// LAYER 3: UART DEBUG
//====================================================
void uart_init(void){
   RCC_APB2ENR |= (1 << 14) | (1 << 2);
   GPIOA_CRH &= ~(0x0F << 4); //GPIOA9 => (9-8)*4=4
   GPIOA_CRH |= (0x0A << 4);
   
   USART1_BRR = 833;
   USART1_CR1 |= (1 << 3);
   USART1_CR1 |= (1 << 13);

}

void uart_send_char(char c){
   while(!(USART1_SR & (1 << 7))); // chờ TXE trống
   USART1_DR = c;
}

void uart_send_string(const char* s){
   while(*s) uart_send_char(*s++);
}

void uart_send_num(uint8_t n){
   uart_send_char('0' + n/10);
   uart_send_char('0' + n%10);
}

//=============================================
//MAIN
//=============================================

int main(void){
   uart_init();
   i2c_init();

   uart_send_string("DS3231 init oke\r\n");

   ds3231_set_time(10, 30, 0);

   for(volatile int i =0; i < 500000; i ++);

   while(1){
      DS3231_Time t = ds3231_get_time();

      uart_send_num(t.hours);
      uart_send_char(':');
      uart_send_num(t.minutes);
      uart_send_char(':');
      uart_send_num(t.seconds);
      uart_send_string("\r\n");

      for(volatile int i=0; i<200000; i++);
   }
}
