/**
 * @file    drivers/si7020.h
 * @brief   Driver for SI7020 humidity and temperature sensor
 */

#ifndef __DRIVERS_SI7020_H
#define __DRIVERS_SI7020_H

#include <types.h>

typedef struct {
    uint8_t i2c_device;
} si7020_desc_t;

/**
 * Read temperature
 *
 * @param desc  Device descriptor
 * @return temperature in milli degrees C (or 0 if not responding)
 */
extern int32_t SI7020_ReadTempmDeg(const si7020_desc_t *desc);

/**
 * Read relative humidity
 *
 * @param desc  Device descriptor
 * @return RH in percent, or 0 if not responding
 */
extern uint8_t SI7020_RH(const si7020_desc_t *desc);

/**
 * Initialize the module
 *
 * @param [out] desc    Device descriptor
 * @param i2c_device    I2C device the sensor is connected to
 * @return true if sensor responded, false otherwise
 */
extern bool SI7020_Init(si7020_desc_t *desc, uint8_t i2c_device);

#endif
