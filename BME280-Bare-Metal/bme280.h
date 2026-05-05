#ifndef BME280_H
#define BME280_H

#include <stdint.h>

// =================================================================
// SECTION 1: DEVICE IDENTITY & REGISTER MAP
// =================================================================
#define BME280_I2C_ADDR       0x76
#define BME280_CHIP_ID_VALUE  0x60

#define BME280_REG_ID           0xD0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_CALIB_T1_BASE 0x88
#define BME280_REG_CALIB_H1     0xA1
#define BME280_REG_CALIB_H2_BASE 0xE1
#define BME280_REG_DATA_START   0xF7

// =================================================================
// SECTION 2: CONFIGURATION MASKS
// =================================================================

/* CTRL_HUM (0xF2) — humidity oversampling x1 */
#define BME280_OSRS_H        0x01

/* CTRL_MEAS (0xF4) */
#define BME280_OSRS_P        (0x01 << 2)
#define BME280_OSRS_T        (0x01 << 5)
#define BME280_MODE_NORMAL   0x03

/* CONFIG (0xF5) — IIR filter coefficient 2 */
#define BME280_FILTER_OFF    0x00
#define BME280_FILTER        (0x02 << 2)

// =================================================================
// SECTION 3: DATA STRUCTURES
// =================================================================

typedef int32_t  BME280_S32_t;
typedef uint32_t BME280_U32_t;
typedef int64_t  BME280_S64_t;

typedef struct {
    /* Temperature */
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    /* Pressure */
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    /* Humidity */
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} BME280_Calib_t;

typedef struct {
    uint32_t press;
    uint32_t temp;
    uint32_t hum;
} BME280_raw_t;

// =================================================================
// SECTION 4: PUBLIC API
// =================================================================

/**
 * @brief  Verify the BME280 chip ID over I2C.
 * @return 1 if device found, 0 otherwise.
 */
uint8_t bme280_chip_verify(void);

/**
 * @brief  Read all factory calibration data from the sensor.
 */
void bme280_read_calib(BME280_Calib_t *cal);

/**
 * @brief  Write operating mode and oversampling settings.
 */
void bme280_config(void);

/**
 * @brief  Burst-read 8 raw ADC bytes (press, temp, hum) from 0xF7.
 *         Blocks until the sensor measurement-in-progress flag clears.
 * @param  p_raw  Pointer to raw data struct to populate.
 */
void bme280_read_raw(BME280_raw_t *p_raw);

/**
 * @brief  Compensate raw temperature (datasheet §4.2.3).
 * @return Temperature in 0.01 °C units (e.g. 5123 = 51.23 °C).
 *         Also sets the internal t_fine variable used by P/H compensation.
 */
BME280_S32_t BME280_compensate_T_int32(BME280_Calib_t *p_calib,
                                        BME280_S32_t adc_T);

/**
 * @brief  Compensate raw pressure (datasheet §4.2.3).
 * @return Pressure in Q24.8 Pa (divide by 256 for Pa, by 25600 for hPa).
 */
BME280_U32_t BME280_compensate_P_int64(BME280_Calib_t *p_calib,
                                        BME280_S32_t adc_P);

/**
 * @brief  Compensate raw humidity (datasheet §4.2.3).
 * @return Humidity in Q22.10 %RH (divide by 1024 for %RH).
 */
BME280_U32_t bme280_compensate_H_int32(BME280_Calib_t *p_calib,
                                        BME280_S32_t adc_H);

#endif /* BME280_H */
