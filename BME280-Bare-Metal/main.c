/**
 * @file    main.c
 * @brief   Bare-metal BME280 driver on STM32F103 (Blue Pill)
 *          Reads temperature, pressure, humidity over I2C
 *          and transmits formatted data via UART1 at 9600 baud.
 *
 * @hardware STM32F103C8T6 @ 8 MHz HSI
 *           BME280 on I2C1 (PB6=SCL, PB7=SDA), addr 0x76
 *           UART1 TX on PA9
 *
 * @author  Tran Anh Quan
 * @date    2026-05-05
 */
#include "systick.h"
#include "uart.h"
#include "i2c.h"
#include "bme280.h"

// =================================================================
// LAYER 5: MAIN
// =================================================================

int main(void) {
    Systick_init();
    uart_init();
    i2c_init();

    uart_send_string("--- BME280 SYSTEM STARTING ---\r\n");

    if (!bme280_chip_verify()) {
        uart_send_string("[ERROR] BME280 Sensor not found! System Halted.\r\n");
        while (1);
    }
    uart_send_string("[OK] BME280 Found.\r\n");

    BME280_Calib_t calib;
    bme280_read_calib(&calib);
    uart_send_string("[OK] Calibration data loaded.\r\n");

    bme280_config();
    uart_send_string("[OK] Sensor configured. Entering Loop...\r\n");

    BME280_raw_t raw;

    while (1) {
        bme280_read_raw(&raw);

        int32_t  temp_final  = BME280_compensate_T_int32(&calib, (int32_t)raw.temp);
        uint32_t press_final = BME280_compensate_P_int64(&calib, (int32_t)raw.press);
        uint32_t hum_final   = bme280_compensate_H_int32(&calib, (int32_t)raw.hum);

        uart_send_string("T: ");
        uart_send_fixed(temp_final, 2);
        uart_send_string(" C | ");

        uart_send_string("P: ");
        uart_send_fixed((int32_t)(press_final / 256U), 2);
        uart_send_string(" hPa | ");

        uart_send_string("H: ");
        uart_send_bme280_humidity(hum_final);
        uart_send_string("\r\n");

        delay_ms(1000);
    }
}
