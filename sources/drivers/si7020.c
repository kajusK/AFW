/**
 * @file    drivers/si7020.c
 * @brief   Driver for SI7020 humidity and temperature sensor
 *
 * https://www.silabs.com/documents/public/data-sheets/Si7020-A20.pdf
 */

#include <types.h>
#include <hal/i2c.h>
#include <utils/time.h>
#include "drivers/si7020.h"

#define SI7020_ADDR 0x40

#define CMD_RESET 0xfe
#define CMD_MEASURE_RH 0xe5
#define CMD_MEASURE_TEMP 0xe3
#define CMD_READ_TEMP 0xe0  /* read temperature from previous RH measurement */

int32_t SI7020_ReadTempmDeg(const si7020_desc_t *desc)
{
    uint8_t cmd = CMD_MEASURE_TEMP;
    bool ret;
    uint16_t data;

    /* uses clock stretching to wait until measurement is finished */
    ret = I2Cd_Transceive(desc->i2c_device, SI7020_ADDR, &cmd, 1,
            (uint8_t *)&data, 2);
    if (!ret) {
        return 0;
    }

    return 175720*(uint32_t)data/65536 - 46850;
}

uint8_t SI7020_RH(const si7020_desc_t *desc)
{
    uint8_t cmd = CMD_MEASURE_RH;
    bool ret;
    uint16_t data;
    uint8_t res;

    /* uses clock stretching to wait until measurement is finished */
    ret = I2Cd_Transceive(desc->i2c_device, SI7020_ADDR, &cmd, 1,
            (uint8_t *)&data, 2);
    if (!ret) {
        return 0;
    }

    res = (125*(uint32_t) data)/65536 - 6;
    if (res > 100) {
        res = 100;
    }
    return res;
}

bool SI7020_Init(si7020_desc_t *desc, uint8_t i2c_device)
{
    ASSERT_NOT(desc == NULL);
    uint8_t cmd = CMD_RESET;
    desc->i2c_device = i2c_device;

    return I2Cd_Transceive(desc->i2c_device, SI7020_ADDR, &cmd, 1, NULL, 0);
}
