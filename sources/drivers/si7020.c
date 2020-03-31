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
 * @file    drivers/si7020.c
 * @brief   Driver for SI7020 humidity and temperature sensor
 *
 * https://www.silabs.com/documents/public/data-sheets/Si7020-A20.pdf
 *
 * @addtogroup drivers
 * @{
 */

#include <hal/i2c.h>
#include <utils/time.h>

#include "drivers/si7020.h"

#define I2C_DEVICE 1
#define SI7020_ADDR 0x40

#define CMD_RESET 0xfe
#define CMD_MEASURE_RH 0xe5
#define CMD_MEASURE_TEMP 0xe3
#define CMD_READ_TEMP 0xe0  /* read temperature from previous RH measurement */

int32_t SI7020_ReadTempmDeg(void)
{
    uint8_t cmd = CMD_MEASURE_TEMP;
    bool ret;
    uint16_t data;

    /* uses clock stretching to wait until measurement is finished */
    ret = I2Cd_Transceive(I2C_DEVICE, SI7020_ADDR, &cmd, 1, (uint8_t *)&data, 2);
    if (!ret) {
        return 0;
    }

    return 175720*(uint32_t)data/65536 - 46850;
}

uint8_t SI7020_RH(void)
{
    uint8_t cmd = CMD_MEASURE_RH;
    bool ret;
    uint16_t data;
    uint8_t res;

    /* uses clock stretching to wait until measurement is finished */
    ret = I2Cd_Transceive(I2C_DEVICE, SI7020_ADDR, &cmd, 1, (uint8_t *)&data, 2);
    if (!ret) {
        return 0;
    }

    res = (125*(uint32_t) data)/65536 - 6;
    if (res > 100) {
        res = 100;
    }
    return res;
}

bool SI7020_Init(void)
{
    uint8_t cmd = CMD_RESET;

    return I2Cd_Transceive(I2C_DEVICE, SI7020_ADDR, &cmd, 1, NULL, 0);
}


/** @} */
