/*
 * Copyright (C) 2020 Jakub Kaderka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file    drivers/ms5607.h
 * @brief   Driver for MS5607 barometric pressure sensor
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_MS5607_H
#define __DRIVERS_MS5607_H

#include <types.h>

/** Sensor I2C address, last bit is inverted value of the CSB pin */
#define MS5607_ADDR_1 0x77
#define MS5607_ADDR_2 0x76

/** Measurement oversamling - higher value = higer precision */
typedef enum {
    MS5607_OSR256,
    MS5607_OSR512,
    MS5607_OSR1024,
    MS5607_OSR2048,
    MS5607_OSR4096,
} ms5607_osr_t;

/** Descriptor for the selected device */
typedef struct {
    uint8_t i2c_device;     /**< I2C device to use */
    uint8_t address;        /**< The MS5607 I2C address to use */
    uint16_t calib[6];      /**< Calibration data of the MS5607 */
} ms5607_desc_t;

/**
 * Read temperature and pressure from the sensor
 *
 * @param desc              Device descriptor
 * @param [out] pressure_mbar   Measured pressure in Pascals or NULL
 * @param [out] temp_mdeg       Measured temperature in milli degrees or NULL
 *
 * @return True if device responded and data are valid
 */
extern bool MS5607_Read(const ms5607_desc_t *desc, ms5607_osr_t osr,
        uint32_t *pressure_Pa, int32_t *temp_mdeg);

/**
 * Initialize the pressure sensor
 *
 * @param [out] desc        Device descriptor
 * @param i2c_device        ID of the I2C port the device is connected to
 * @param address           Address of the device on I2C bus;
 * @return True if successfull, false if not responding
 */
extern bool MS5607_Init(ms5607_desc_t *desc, uint8_t i2c_device,
        uint8_t address);

#endif

/** @} */
