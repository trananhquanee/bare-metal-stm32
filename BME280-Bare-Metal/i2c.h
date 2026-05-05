#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// =================================================================
// I2C STATUS CODES
// =================================================================
typedef enum {
    I2C_OK           = 0,
    I2C_ERROR        = 1,
    I2C_STAT_TIMEOUT = 2,
} I2C_Status;

// =================================================================
// PUBLIC API
// =================================================================

/**
 * @brief  Initialize I2C1 peripheral (PB6=SCL, PB7=SDA, 100 kHz standard mode).
 */
void i2c_init(void);

/**
 * @brief  Write a single byte to a device register.
 */
void i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t data);

/**
 * @brief  Read a single byte from a device register.
 * @return Register value.
 */
uint8_t i2c_read_reg(uint8_t addr, uint8_t reg);

/* ---- Low-level primitives (used by BME280 burst reads) ---- */
I2C_Status i2c_start(void);
void       i2c_stop(void);
I2C_Status i2c_send_addr(uint8_t addr, uint8_t rw);
void       i2c_write_byte(uint8_t data);
uint8_t    i2c_read_byte_ack(void);
uint8_t    i2c_read_byte_nack(void);

#endif /* I2C_H */
