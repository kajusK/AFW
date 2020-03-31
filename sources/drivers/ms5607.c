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
 * https://www.parallax.com/sites/default/files/downloads/29124-MS5607-02BA03-Datasheet.pdf
 *
 * @addtogroup drivers
 * @{
 */

#include <hal/i2c.h>
#include <utils/time.h>

#include "drivers/ms5607.h"

#define CMD_RESET 0x1e
#define CMD_CONVERT_D1 0x40
#define CMD_CONVERT_D2 0x50
#define CMD_READ_ADC 0x00
#define CMD_READ_PROM 0xa0

/* Conversion time for osr 256, each step doubles the time */
#define CONVERSION_TIME_MS 1

#define I2C_DEVICE 1

/** Sensor i2c address, last bit is inverted value of the CSB pin */
#define MS5607_ADDR 0x77

/** Sensor calibration data storage */
static uint16_t ms5607i_calib[6];

/**
 * Send command to MS5607
 *
 * @param cmd   Command to be sent
 * @return True if sensor acked the command
 */
static bool MS5607i_Cmd(uint8_t cmd)
{
    return I2Cd_Transceive(I2C_DEVICE, MS5607_ADDR, &cmd, 1, NULL, 0);
}

/**
 * Verify PROM content is valid
 *
 * https://www.parallax.com/sites/default/files/downloads/29124-APPNote_520_C_code.pdf
 *
 * @param prom  Prom content (16 bytes)
 * @return true if crc matches, false otherwise
 */
static bool MS5607i_Crc(uint16_t *buf)
{
    uint32_t rem = 0;
    uint8_t crc = buf[7] & 0x000f;

    buf[7] &= 0xff00;
    for (uint8_t i = 0; i < 16; i++) {
        if (i & 1) {
            rem ^= buf[i/2] & 0x00ff;
        } else {
            rem ^= buf[i/2] >> 8;
        }

        for (uint8_t j = 0; j < 8; j++) {
            if (rem & 0x8000) {
                rem ^= 0x1800;
            }
            rem <<= 1;
        }
    }

    buf[7] |= crc;
    rem = (rem >> 12) & 0x000f;
    if (crc == rem) {
        return true;
    }
    return false;
}

/**
 * Read measured value
 *
 * @param val   Pointer to store result to
 * @return  true if succeeded, false if not responding
 */
static bool MS5607i_ReadAdc(uint32_t *val)
{
    uint8_t buf[3];

    MS5607i_Cmd(CMD_READ_ADC);
    if (!I2Cd_Transceive(I2C_DEVICE, MS5607_ADDR, NULL, 0, buf, 3)) {
        return false;
    }

    *val = buf[0] << 16 | buf[1] << 8 | buf[2];
    return true;
}

/**
 * Read value from PROM
 *
 * @param addr      Address to read from
 * @param val       Pointer to store result to
 * @return  true if succeeded, false if not responding
 */
static bool MS5607i_ReadProm(uint8_t addr, uint16_t *val)
{
    uint8_t buf[2];

    MS5607i_Cmd(CMD_READ_PROM | (addr << 1));
    if (!I2Cd_Transceive(I2C_DEVICE, MS5607_ADDR, NULL, 0, buf, 2)) {
        return false;
    }

    *val = buf[0] << 8 | buf[1];
    return true;
}

bool MS5607_Read(ms5607_osr_t osr, uint32_t *pressure_Pa, int32_t *temp_mdeg)
{
    uint32_t d1, d2;
    int32_t dt, temp, p, t2;
    int64_t off, sens, off2, sens2;
    int64_t tmp;

    MS5607i_Cmd(CMD_CONVERT_D1 | osr);
    delay_ms(CONVERSION_TIME_MS << osr);
    if (!MS5607i_ReadAdc(&d1)) {
        return false;
    }

    MS5607i_Cmd(CMD_CONVERT_D2 | osr);
    delay_ms(CONVERSION_TIME_MS << osr);
    if (!MS5607i_ReadAdc(&d2)) {
        return false;
    }

    dt = (int32_t)d2 - ((uint32_t)ms5607i_calib[4] << 8);
    temp = 2000 + ((dt*(int64_t)ms5607i_calib[5]) >> 23);

    off = ((uint64_t) ms5607i_calib[1] << 17) + (((int64_t)ms5607i_calib[3]*dt) >> 6);
    sens = ((uint64_t) ms5607i_calib[0] << 16) + (((int64_t)ms5607i_calib[2]*dt) >> 7);

    if (temp < 2000) {
        t2 = ((int64_t)dt*dt) >> 31;
        tmp = ((int64_t) temp - 2000)*(temp - 2000);
        off2 = (61*tmp) >> 4;
        sens2 = 2*tmp;
        if (temp < -1500) {
            tmp = ((int64_t) temp + 1500)*(temp + 1500);
            off2 += 15*tmp;
            sens2 += 8*tmp;
        }
    } else {
        t2 = 0;
        off2 = 0;
        sens2 = 0;
    }

    temp = temp - t2;
    off = off - off2;
    sens = sens - sens2;
    p = (((d1*sens) >> 21) - off) >> 15;

    *temp_mdeg = temp * 10;
    *pressure_Pa = p; /* result in mbar * 100, same as Pa */

    return true;
}

bool MS5607_Init(void)
{
    uint16_t buf[8];

    if (!MS5607i_Cmd(CMD_RESET)) {
        return false;
    }
    delay_ms(4);

    for (uint8_t i = 0; i < 8; i++) {
        MS5607i_ReadProm(i, &buf[i]);
    }

    if (!MS5607i_Crc(buf)) {
        return false;
    }

    /* finally copy valid calibration to global variable */
    for (uint8_t i = 0; i < 6; i++) {
        ms5607i_calib[i] = buf[i+1];
    }

    return true;
}

/** @} */
