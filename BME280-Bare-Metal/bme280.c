#include "bme280.h"
#include "i2c.h"
#include "uart.h"

// =================================================================
// LAYER 4: BME280 DRIVER IMPLEMENTATION
// =================================================================

/* t_fine is shared between temperature, pressure, and humidity
 * compensation formulas (datasheet §4.2.3). */
static BME280_S32_t t_fine = 0;

// -----------------------------------------------------------------
// Chip verification
// -----------------------------------------------------------------
uint8_t bme280_chip_verify(void) {
    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 0);
    i2c_write_byte(BME280_REG_ID);

    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 1);
    uint8_t chip_id = i2c_read_byte_nack();

    return (chip_id == BME280_CHIP_ID_VALUE) ? 1U : 0U;
}

// -----------------------------------------------------------------
// Calibration data
// -----------------------------------------------------------------
void bme280_read_calib(BME280_Calib_t *cal) {
    uint8_t buf1[24];
    uint8_t buf2[7];

    /* --- Temperature & Pressure calibration (0x88 … 0x9F) --- */
    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 0);
    i2c_write_byte(0x88);

    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 1);
    for (int i = 0; i < 23; i++) buf1[i] = i2c_read_byte_ack();
    buf1[23] = i2c_read_byte_nack();

    cal->dig_T1 = (uint16_t)((buf1[1] << 8) | buf1[0]);
    cal->dig_T2 = (int16_t) ((buf1[3] << 8) | buf1[2]);
    cal->dig_T3 = (int16_t) ((buf1[5] << 8) | buf1[4]);

    cal->dig_P1 = (uint16_t)((buf1[7]  << 8) | buf1[6]);
    cal->dig_P2 = (int16_t) ((buf1[9]  << 8) | buf1[8]);
    cal->dig_P3 = (int16_t) ((buf1[11] << 8) | buf1[10]);
    cal->dig_P4 = (int16_t) ((buf1[13] << 8) | buf1[12]);
    cal->dig_P5 = (int16_t) ((buf1[15] << 8) | buf1[14]);
    cal->dig_P6 = (int16_t) ((buf1[17] << 8) | buf1[16]);
    cal->dig_P7 = (int16_t) ((buf1[19] << 8) | buf1[18]);
    cal->dig_P8 = (int16_t) ((buf1[21] << 8) | buf1[20]);
    cal->dig_P9 = (int16_t) ((buf1[23] << 8) | buf1[22]);

    /* --- Humidity calibration H1 (0xA1) --- */
    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 0);
    i2c_write_byte(0xA1);

    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 1);
    cal->dig_H1 = i2c_read_byte_nack();

    /* --- Humidity calibration H2-H6 (0xE1 … 0xE7) --- */
    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 0);
    i2c_write_byte(0xE1);

    i2c_start();
    i2c_send_addr(BME280_I2C_ADDR, 1);
    for (int j = 0; j < 6; j++) buf2[j] = i2c_read_byte_ack();
    buf2[6] = i2c_read_byte_nack();

    cal->dig_H2 = (int16_t) ((buf2[1] << 8) | buf2[0]);
    cal->dig_H3 = buf2[2];
    cal->dig_H4 = (int16_t) ((buf2[3] << 4) | (buf2[4] & 0x0F));
    cal->dig_H5 = (int16_t) ((buf2[5] << 4) | (buf2[4] >> 4));
    cal->dig_H6 = (int8_t)   buf2[6];
}

// -----------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------
void bme280_config(void) {
    /* ctrl_hum must be written before ctrl_meas to take effect */
    i2c_write_reg(BME280_I2C_ADDR, BME280_REG_CTRL_HUM,  BME280_OSRS_H);
    i2c_write_reg(BME280_I2C_ADDR, BME280_REG_CONFIG,    BME280_FILTER);
    i2c_write_reg(BME280_I2C_ADDR, BME280_REG_CTRL_MEAS,
                  BME280_OSRS_T | BME280_OSRS_P | BME280_MODE_NORMAL);
}

// -----------------------------------------------------------------
// Raw data burst read
// -----------------------------------------------------------------
void bme280_read_raw(BME280_raw_t *p_raw) {
    uint8_t buf[8];

    /* Wait until measurement is complete (status bit 3 = measuring) */
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

    for (uint8_t t = 0; t < 7; t++) buf[t] = i2c_read_byte_ack();
    buf[7] = i2c_read_byte_nack();

    p_raw->press = ((uint32_t)buf[0] << 12) | ((uint32_t)buf[1] << 4) | (buf[2] >> 4);
    p_raw->temp  = ((uint32_t)buf[3] << 12) | ((uint32_t)buf[4] << 4) | (buf[5] >> 4);
    p_raw->hum   = ((uint32_t)buf[6] << 8)  |  (uint32_t)buf[7];
}

// -----------------------------------------------------------------
// Compensation formulas — from BME280 datasheet §4.2.3
// -----------------------------------------------------------------

BME280_S32_t BME280_compensate_T_int32(BME280_Calib_t *p_calib,
                                        BME280_S32_t adc_T) {
    BME280_S32_t var1, var2, T;

    var1 = ((((adc_T >> 3) - ((BME280_S32_t)p_calib->dig_T1 << 1)))
            * ((BME280_S32_t)p_calib->dig_T2)) >> 11;

    var2 = ((((adc_T >> 4) - ((BME280_S32_t)p_calib->dig_T1))
             * ((adc_T >> 4) - ((BME280_S32_t)p_calib->dig_T1))) >> 12)
           * ((BME280_S32_t)p_calib->dig_T3) >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

BME280_U32_t BME280_compensate_P_int64(BME280_Calib_t *p_calib,
                                        BME280_S32_t adc_P) {
    BME280_S64_t var1, var2, p;

    var1 = ((BME280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BME280_S64_t)p_calib->dig_P6;
    var2 = var2 + ((var1 * (BME280_S64_t)p_calib->dig_P5) << 17);
    var2 = var2 + (((BME280_S64_t)p_calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (BME280_S64_t)p_calib->dig_P3) >> 8)
         + ((var1 * (BME280_S64_t)p_calib->dig_P2) << 12);
    var1 = (((((BME280_S64_t)1) << 47) + var1))
         * ((BME280_S64_t)p_calib->dig_P1) >> 33;

    if (var1 == 0) return 0; /* avoid division by zero */

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((BME280_S64_t)p_calib->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((BME280_S64_t)p_calib->dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BME280_S64_t)p_calib->dig_P7) << 4);

    return (BME280_U32_t)p;
}

BME280_U32_t bme280_compensate_H_int32(BME280_Calib_t *p_calib,
                                        BME280_S32_t adc_H) {
    BME280_S32_t v_x1_u32r;

    v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
    v_x1_u32r = (((((adc_H << 14)
        - (((BME280_S32_t)p_calib->dig_H4) << 20)
        - (((BME280_S32_t)p_calib->dig_H5) * v_x1_u32r))
        + ((BME280_S32_t)16384)) >> 15)
        * (((((((v_x1_u32r * ((BME280_S32_t)p_calib->dig_H6)) >> 10)
        * (((v_x1_u32r * ((BME280_S32_t)p_calib->dig_H3)) >> 11)
        + ((BME280_S32_t)32768))) >> 10)
        + ((BME280_S32_t)2097152))
        * ((BME280_S32_t)p_calib->dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r
        - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7)
        * ((BME280_S32_t)p_calib->dig_H1)) >> 4));

    v_x1_u32r = (v_x1_u32r < 0         ?         0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (BME280_U32_t)(v_x1_u32r >> 12);
}
