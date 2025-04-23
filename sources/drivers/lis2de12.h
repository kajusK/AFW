/**
 * @file    drivers/lis2de12.h
 * @brief   Driver for ST LIS2DE12 accelerometer
 * @note    Will most likely work for other LIS2* accelerometers, not tested
 */

#ifndef __DRIVERS_LIS2DE12_H
#define __DRIVERS_LIS2DE12_H

#include <types.h>

/** Sensor I2C address, last bit is inverted value of the SDO pin */
#define LIS2DE12_ADDR_1 0x18
#define LIS2DE12_ADDR_2 0x19

/** Measuring rate */
typedef enum {
    LIS2DE12_ODR_1HZ = 0x01,
    LIS2DE12_ODR_10HZ = 0x02,
    LIS2DE12_ODR_25HZ = 0x03,
    LIS2DE12_ODR_50HZ = 0x04,
    LIS2DE12_ODR_100HZ = 0x05,
    LIS2DE12_ODR_200HZ = 0x06,
    LIS2DE12_ODR_400HZ = 0x07,
    LIS2DE12_ODR_1620HZ = 0x08,
    LIS2DE12_ODR_5376HZ = 0x09,
} lis2de12_odr_t;

/** Range of the measurement */
typedef enum {
    LIS2DE12_SCALE_2G = 0x00,
    LIS2DE12_SCALE_4G = 0x01,
    LIS2DE12_SCALE_8G = 0x02,
    LIS2DE12_SCALE_16G = 0x03,
} lis2de12_scale_t;

/** Device descriptor */
typedef struct {
    uint8_t i2c_device;     /**< I2C device to use */
    uint8_t address;        /**< The MS5607 I2C address to use */
    lis2de12_odr_t odr;     /**< Currently set ODR */
    lis2de12_scale_t scale; /**< Currently set measure scale */
} lis2de12_desc_t;

/**
 * Read acceleration data
 *
 * @param desc  Device descriptor
 * @param x_mg  Acceleration in X axis in milli G, NULL if not needed
 * @param y_mg  Acceleration in Y axis in milli G, NULL if not needed
 * @param z_mg  Acceleration in Z axis in milli G, NULL if not needed
 * @return True on success, false if data not available
 */
bool LIS2DE12_GetAccel(const lis2de12_desc_t *desc, int16_t *x_mg, int16_t *y_mg, int16_t *z_mg);

/**
 * Power on the accelerometer and start measuring
 *
 * @param desc  Device descriptor
 */
void LIS2DE12_PowerOn(const lis2de12_desc_t *desc);

/**
 * Put device into power off state (still responds on I2C)
 *
 * @param desc  Device descriptor
 */
void LIS2DE12_PowerOff(const lis2de12_desc_t *desc);

/**
 * Configure measurement mode
 *
 * @param desc  Device descriptor
 * @param odr   Sample rate
 * @param scale Measurement scale
 */
void LIS2DE12_Configure(lis2de12_desc_t *desc, lis2de12_odr_t odr, lis2de12_scale_t scale);

/**
 * Get and clear interrupt flag
 *
 * @param desc  Device descriptor
 * @return True if interrupt active
 */
bool LIS2DE12_GetClearIntFlag(const lis2de12_desc_t *desc);

/**
 * Disable interrupt generation on INT1
 *
 * @param desc  Device descriptor
 */
void LIS2DE12_DisableInt(const lis2de12_desc_t *desc);

/**
 * Enable interrupt generation on INT1
 *
 * @param desc  Device descriptor
 * @param threshold_mg  Interrupt acceleration threshold in mG
 */
void LIS2DE12_EnableInt(const lis2de12_desc_t *desc, uint16_t threshold_mg);

/**
 * Initialize the sensor
 *
 * @param desc        Device descriptor
 * @param i2c_device  ID of the I2C port the device is connected to
 * @param address     Address of the device on I2C bus
 * @return True if successfull, false if not responding
 */
bool LIS2DE12_Init(lis2de12_desc_t *desc, uint8_t i2c_device, uint8_t address);

#endif
