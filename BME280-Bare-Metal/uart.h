#ifndef UART_H
#define UART_H

#include <stdint.h>

// =================================================================
// PUBLIC API
// =================================================================

/**
 * @brief  Initialize USART1 (PA9 TX, 9600 baud, 8N1).
 */
void uart_init(void);

void uart_send_char(char c);
void uart_send_string(const char *s);

/**
 * @brief  Send a signed 32-bit integer as ASCII digits.
 */
void uart_send_int(int32_t n);

/**
 * @brief  Send a signed integer with a fixed decimal point.
 *         e.g. uart_send_fixed(2945, 2) → "29.45"
 */
void uart_send_fixed(int32_t n, uint8_t decimals);

/**
 * @brief  Send BME280 humidity from Q22.10 raw format.
 *         e.g. h_raw = 70656 → "69.00%"
 */
void uart_send_bme280_humidity(uint32_t h_raw);

#endif /* UART_H */
