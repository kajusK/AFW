/**
 * @file    drivers/scd4x.h
 * @brief   Sensirion SCD4x CO2 sensor
 *
 * https://sensirion.com/media/documents/E0F04247/631EF271/CD_DS_SCD40_SCD41_Datasheet_D1.pdf
 */

#ifndef __DRIVERS_SCD4X_H
#define __DRIVERS_SCD4X_H

#include <types.h>

/** The device descriptor */
typedef struct {
    uint8_t i2c_device;
} scd4x_desc_t;

/**
 * Start periodic measurement mode
 *
 * Result is available every 5 seconds, or every 30 seconds in low power mode
 *
 * @param desc
 * @param low_power Reduce measurement period if true
 * @return Operation result, false if failed
 */
bool SCD4x_StartPeriodic(const scd4x_desc_t *desc, bool low_power);

/**
 * Run a single shot measurement (takes 5 seconds)
 *
 * Only available at SCD41
 *
 * @param desc
 * @return Operation result, false if failed
 */
bool SCD4x_SingleShot(const scd4x_desc_t *desc);

/**
 * Stop periodic measurement mode
 *
 * @param desc
 * @return Operation result, false if failed
 */
bool SCD4x_StopPeriodic(const scd4x_desc_t *desc);

/**
 * Read measured data
 *
 * The measurement must finish before reading, else false is returned.
 * Optionally call IsMeasReady to check the measurement status.
 * The first measurement after power up shall be discarded.
 *
 * @param desc
 * @param ppm   CO2 concentration in ppm or NULL if not interested
 * @param temp  temperature in degrees or NULL if not interested
 * @param rh    Relative humidity or NULL if not interested
 *
 * @return True if data read successfully, false if not ready or not responding
 */
bool SCD4x_ReadData(const scd4x_desc_t *desc, uint16_t *ppm, int16_t *temp, uint8_t *rh);

/**
 * Check if data are ready to be read
 *
 * @param desc
 * @return True if ready, false otherwise
 */
bool SCD4x_IsMeasReady(const scd4x_desc_t *desc);

/**
 * Set current ambient pressure to increase sensor accuracy
 *
 * @param desc
 * @param pressure_pa   Current ambient pressure in Pa
 * @return Operation result, false if failed
 */
bool SCD4x_SetPressure(const scd4x_desc_t *desc, uint32_t pressure_pa);

/**
 * Power the sensor down
 *
 * @param desc
 * @return Operation result, false if failed
 */
bool SCD4x_PowerDown(const scd4x_desc_t *desc);

/**
 * Wake Up the sensor from power off mode (20 ms duration)
 *
 * @param desc
 * @return Operation result, false if failed
 */
bool SCD4x_WakeUp(const scd4x_desc_t *desc);

/**
 * Initialize the SCD4x device
 *
 * It could take up to 1 second for sensor to boot up - delay loop inside!
 *
 * @param desc
 * @param i2c_device    Number of i2c device to be used (only 100 kHz mode supported by sensor)
 * @return True if ready, False is not responding
 */
bool SCD4x_Init(scd4x_desc_t *desc, uint8_t i2c_device);

#endif
