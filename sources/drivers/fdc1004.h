/**
 * @file    drivers/fdc1004.h
 * @brief   Driver for FDC1004 capacitance to digital converter
 */

#ifndef __DRIVERS_FDC1004_H
#define __DRIVERS_FDC1004_H

#include <types.h>

/** Measurement sample rate, more samples = slower but more precise */
typedef enum {
    FDC_RATE_100S = 0x01,
    FDC_RATE_200S = 0x02,
    FDC_RATE_400S = 0x03
} fdc1004_rate_t;

/** Measurement channel */
typedef enum {
    FDC_MEAS_1 = 0x01,
    FDC_MEAS_2 = 0x02,
    FDC_MEAS_3 = 0x04,
    FDC_MEAS_4 = 0x08,
} fdc1004_meas_t;

/** Channel selection */
typedef enum {
    FDC_CIN1 = 0x01,
    FDC_CIN2 = 0x02,
    FDC_CIN3 = 0x03,
    FDC_CIN4 = 0x04,
    FDC_CAPDAC = 0x05,
    FDC_DISABLED = 0x07,
} fdc1004_ch_t;

/** Device descriptor */
typedef struct {
    uint8_t i2c_device; /**< I2C peripheral number */
} fdc1004_desc_t;

/**
 * Read measurement result from the sensor
 *
 * @param desc      The device descriptor
 * @param channel   Channel to read result for
 * @param raw       Raw reading of the sensor data
 * @return True if sensor acked the command
 */
bool FDC1004_ReadResultRaw(const fdc1004_desc_t *desc, fdc1004_meas_t channel, uint32_t *raw);

/**
 * Check if the measurement is already finished (have valid data)
 *
 * @param desc      Device descriptor
 * @param channel   Measurement channel to check
 * @return True if have valid data ready
 */
bool FDC1004_IsMeasComplete(const fdc1004_desc_t *desc, fdc1004_meas_t channel);

/**
 * Configure measurement
 *
 * @param desc      Device descriptor
 * @param channel   Measurement channel
 * @param positive  Positive input (only physical channels)
 * @param negative  Negative input (channels, capdac, disabled),
 *                  must be higher then positive channel number
 * @param offset_pf Offset in pF to apply on the measurements
 * @return True if sensor acked the command
 */
bool FDC1004_ConfigureMeasurement(const fdc1004_desc_t *desc, fdc1004_meas_t channel,
    fdc1004_ch_t positive, fdc1004_ch_t negative, uint32_t offset_pf);

/**
 * Run single measurement
 *
 * @param desc      Device descriptor
 * @param rate      Sample rate to use
 * @param channel   Measurement channel to start conversion for
 * @param True if measurement started
 */
bool FDC1004_RunSingle(const fdc1004_desc_t *desc, fdc1004_rate_t rate, fdc1004_meas_t channel);

/**
 * Start repeated measurements
 *
 * @param desc      Device descriptor
 * @param rate      Sample rate (affects precision)
 * @param channels  List of channels to measure (e.g. FDC_MEAS_1 | FDC_MEAS_2)
 * @param True if measurement started
 */
bool FDC1004_RunRepeated(const fdc1004_desc_t *desc, fdc1004_rate_t rate, uint8_t channels);

/**
 * Initialize the device driver
 *
 * @param desc      Device descriptor to store config to
 * @param i2c_device    I2C peripheral identification
 *
 * @return True if sensor is responding
 */
bool FDC1004_Init(fdc1004_desc_t *desc, uint8_t i2c_device);

#endif
