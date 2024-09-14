/*
 * Copyright (C) 2024 Jakub Kaderka
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
 * @file    drivers/scd4x.c
 * @brief   Sensirion SCD4x CO2 sensor
 *
 * https://sensirion.com/media/documents/E0F04247/631EF271/CD_DS_SCD40_SCD41_Datasheet_D1.pdf
 *
 * @addtogroup drivers
 * @{
 */

#include <types.h>

#include "hal/i2c.h"
#include "utils/assert.h"
#include "utils/crc.h"
#include "utils/time.h"

#include "drivers/scd4x.h"

#define SCD4x_ADDRESS 0x62

#define CMD_START_PERIODIC 0x21b1
#define CMD_START_LOW_POWER 0x21ac
#define CMD_SINGLE_SHOT 0x219d
#define CMD_READ_MEASUREMENT 0xec05
#define CMD_STOP_PERIODIC 0x3f86
#define CMD_SET_AMBIENT_PRESSURE 0xe000
#define CMD_GET_READY 0xe4b8
#define CMD_GET_SERIAL 0x3682
#define CMD_POWER_OFF 0x36e0
#define CMD_WAKE_UP 0x36f6

/**
 * Read data from the sensor
 *
 * @param desc      The device descriptor
 * @param cmd       Command to be sent
 * @param data      Data buffer
 * @param len       Amount of words to be read, up to 3
 * @return I2C ack arrived or False
 */
static bool read(const scd4x_desc_t *desc, uint16_t cmd, uint16_t *data, uint8_t len)
{
    uint8_t buf[9];
    bool res;

    ASSERT(len <= 3);
    
    buf[0] = (cmd & 0xff00) >> 8;
    buf[1] = cmd & 0x00ff;
    res = I2Cd_Transceive(desc->i2c_device, SCD4x_ADDRESS, buf, 2, buf, len*3);
    if (!res) {
        return false;
    }

    for (uint8_t i = 0; i < len*3; i += 3) {
        if (CRC8(&buf[i], 2) != buf[i+2]) {
            return false;
        }
        *data++ = buf[i] << 8 | buf[i+1];
    }
    return true;
}

/**
 * Write data to the sensor
 *
 * @param desc      The device descriptor
 * @param cmd       Command to be sent
 * @param data      Data to write or NULL
 * @return I2C ack arrived or False
 */
static bool write(const scd4x_desc_t *desc, uint16_t cmd, const uint16_t *data)
{
    uint8_t buf[5];
    uint8_t len = 2;
    buf[0] = (cmd & 0xff00) >> 8;
    buf[1] = cmd & 0x00ff;

    if (data != NULL) {
        buf[2] = ((*data) & 0xff00) >> 8;
        buf[3] = (*data) & 0x00ff;
        buf[4] = CRC8(&buf[2], 2);
        len = 5;
    }

    return I2Cd_Transceive(desc->i2c_device, SCD4x_ADDRESS, buf, len, NULL, 0);
}

bool SCD4x_StartPeriodic(const scd4x_desc_t *desc, bool low_power)
{
    if (low_power) {
        return write(desc, CMD_START_LOW_POWER, NULL);
    }
    return write(desc, CMD_START_PERIODIC, NULL);
}

bool SCD4x_SingleShot(const scd4x_desc_t *desc)
{
    return write(desc, CMD_SINGLE_SHOT, NULL);
}

bool SCD4x_StopPeriodic(const scd4x_desc_t *desc)
{
    return write(desc, CMD_STOP_PERIODIC, NULL);
}

bool SCD4x_ReadData(const scd4x_desc_t *desc, uint16_t *ppm, int16_t *temp, uint8_t *rh)
{
    uint16_t buf[3];

    if (!read(desc, CMD_READ_MEASUREMENT, buf, 3)) {
        return false;
    }
    if (ppm != NULL) {
        *ppm = buf[0];
    }
    if (temp != NULL) {
        *temp = -45 + ((uint32_t)175*buf[1])/(65535);
    }
    if (rh != NULL) {
        *rh = ((uint32_t)100*buf[2])/(65535);
    }
    return true;
}

bool SCD4x_IsMeasReady(const scd4x_desc_t *desc)
{
    uint16_t buf;

    if (!read(desc, CMD_GET_READY, &buf, 1)) {
        return false;
    }
    // if the least significant 11 bits are zero, data are not ready
    return (buf & 0x07ff) != 0;
}

bool SCD4x_SetPressure(const scd4x_desc_t *desc, uint32_t pressure_pa)
{
    uint16_t data = pressure_pa / 100;
    return write(desc, CMD_SET_AMBIENT_PRESSURE, &data);
}

bool SCD4x_PowerDown(const scd4x_desc_t *desc)
{
    return write(desc, CMD_POWER_OFF, NULL);
}

bool SCD4x_WakeUp(const scd4x_desc_t *desc)
{
    return write(desc, CMD_WAKE_UP, NULL);
}

bool SCD4x_Init(scd4x_desc_t *desc, uint8_t i2c_device)
{
    uint16_t serial[3];

    ASSERT(desc != NULL);

    desc->i2c_device = i2c_device;

    /* The sensor boots up to 1 second */
    for (int i = 0; i < 10; i++) {
        if (read(desc, CMD_GET_SERIAL, serial, 3)) {
            return true;
        }
        delay_ms(100);
    }
    return false;
}

/** @} */