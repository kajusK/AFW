/**
 * @file    drivers/lps22hh.h
 * @brief   Driver for ST LPS22HH Barometer sensor
 */

#ifndef __DRIVERS_LPS22HH_H
#define __DRIVERS_LPS22HH_H

#include <types.h>

/** Sensor I2C address, last bit is value of the SDO pin */
#define LPS22HH_ADDR_1 0x5C
#define LPS22HH_ADDR_2 0x5D

typedef enum {
    LPS22HH_ODR_OFF = 0x0,
    LSP22HH_ODR_1_HZ = 0x01,
    LSP22HH_ODR_10_HZ = 0x02,
    LSP22HH_ODR_25_HZ = 0x03,
    LSP22HH_ODR_50_HZ = 0x04,
    LSP22HH_ODR_75_HZ = 0x05,
    LSP22HH_ODR_100_HZ = 0x06,
    LSP22HH_ODR_200_HZ = 0x07,
} lps22hh_odr_t;

/** Device descriptor */
typedef struct {
    uint8_t i2c_device; /**< I2C device to use */
    uint8_t address;    /**< The device I2C address to use */
} lps22hh_desc_t;

/**
 * Get measured data
 *
 * @param desc          Device descriptor
 * @param pressure_ps   Pointer to store pressure to [Pa], or NULL if not interested
 * @param temp_milli_c  Pointer to store temperature to [milli C], or NULL if not interested
 * @return True on success, false if not ready
 */
bool LPS22HH_GetData(const lps22hh_desc_t *desc, uint32_t *pressure_pa, int16_t *temp_milli_c);

/**
 * Run a single shot measurement, go to sleep afterwards
 *
 * @param desc          Device descriptor
 * @param pressure_ps   Pointer to store pressure to [Pa], or NULL if not interested
 * @param temp_milli_c  Pointer to store temperature to [milli C], or NULL if not interested
 * @return True on success, false if measurement failed (e.g. timeout)
 */
bool LPS22HH_SingleShot(const lps22hh_desc_t *desc, uint32_t *pressure_pa, int16_t *temp_milli_c);

/**
 * Configure sampling
 *
 * @param desc          Device descriptor
 * @param odr           Sample rate or got to off when LPS22HH_ODR_OFF
 * @param low_noise     Low noise mode (higher consumption, only for odr <= 100 Hz)
 */
void LPS22HH_Configure(const lps22hh_desc_t *desc, lps22hh_odr_t odr, bool low_noise);

/**
 * Initialize the sensor
 *
 * @param desc        Device descriptor
 * @param i2c_device  ID of the I2C port the device is connected to
 * @param address     Address of the device on I2C bus
 * @return True if successfull, false if not responding
 */
bool LPS22HH_Init(lps22hh_desc_t *desc, uint8_t i2c_device, uint8_t address);

#endif
