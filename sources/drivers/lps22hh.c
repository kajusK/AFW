/**
 * @file    drivers/lps22hh.c
 * @brief   Driver for ST LPS22HH Barometer sensor
 */

#include <types.h>
#include <hal/i2c.h>
#include "lps22hh.h"

#define REG_INTERRUPT_CFG 0x0B
#define REG_IF_CTRL       0x0E
#define REG_WHO_AM_I      0x0F
#define REG_CTRL1         0x10
#define REG_CTRL2         0x11
#define REG_CTRL3         0x12
#define REG_FIFO_CTRL     0x13
#define REG_STATUS        0x27
#define REG_PRESS_OUT_XL  0x28
#define REG_TEMP_OUT_L    0x2B

#define DEVICE_ID 0xB3

/* Status register bits */
#define STATUS_TEMP_OVERRUN  0x20
#define STATUS_PRESS_OVERRUN 0x10
#define STATUS_TEMP_READY    0x02
#define STATUS_PRESS_READY   0x01

static uint8_t readReg(const lps22hh_desc_t *desc, uint8_t addr)
{
    uint8_t value;
    I2Cd_Transceive(desc->i2c_device, desc->address, &addr, 1, &value, 1);
    return value;
}

static void writeReg(const lps22hh_desc_t *desc, uint8_t addr, uint8_t value)
{
    uint8_t buf[2];
    buf[0] = addr;
    buf[1] = value;
    I2Cd_Transceive(desc->i2c_device, desc->address, buf, sizeof(buf), NULL, 0);
}

static uint32_t getPressurePa(const lps22hh_desc_t *desc)
{
    uint8_t data[3];
    uint32_t value;
    uint8_t reg = REG_PRESS_OUT_XL;

    bool res = I2Cd_Transceive(desc->i2c_device, desc->address, &reg, 1, data, 3);
    if (!res) {
        return 0;
    }

    value = data[0] | (data[1] << 8) | (data[2] << 16);
    // sensitivity is 4096 per hPa
    return (value * 100) / 4096;
}

static int32_t getTemperatureMilliC(const lps22hh_desc_t *desc)
{
    uint8_t data[2];
    int16_t value;
    uint8_t reg = REG_TEMP_OUT_L;

    bool res = I2Cd_Transceive(desc->i2c_device, desc->address, &reg, 1, data, 2);
    if (!res) {
        return 0;
    }

    value = (int16_t)(data[0] | (data[1] << 8));
    // sensitivity is 100 per C
    return value * 10;
}

bool LPS22HH_GetData(const lps22hh_desc_t *desc, uint32_t *pressure_pa, int16_t *temp_milli_c)
{
    uint8_t status = readReg(desc, REG_STATUS);
    bool ret = false;

    if (status & STATUS_PRESS_READY) {
        uint32_t pressure = getPressurePa(desc); // always read to clear flags
        ret = true;
        if (pressure_pa != NULL) {
            *pressure_pa = pressure;
        }
    }

    if (status & STATUS_TEMP_READY) {
        int16_t temperature = getTemperatureMilliC(desc); // always read to clear flags
        ret = true;
        if (temp_milli_c != NULL) {
            *temp_milli_c = temperature;
        }
    }

    return ret;
}

bool LPS22HH_SingleShot(const lps22hh_desc_t *desc, uint32_t *pressure_pa, int16_t *temp_milli_c)
{
    writeReg(desc, REG_CTRL1, 0x02); // block data update, odr = single shot
    writeReg(desc, REG_CTRL2, 0x11); // address auto increment, start single shot

    while (readReg(desc, REG_CTRL2) & 0x01) {
        ; // wait for measurement to be ready
    }
    return LPS22HH_GetData(desc, pressure_pa, temp_milli_c);
}

void LPS22HH_Configure(const lps22hh_desc_t *desc, lps22hh_odr_t odr, bool low_noise)
{
    writeReg(desc, REG_CTRL1, (odr << 4) | 0x02);
    writeReg(desc, REG_CTRL2, 0x10 | (low_noise << 1));
}

bool LPS22HH_Init(lps22hh_desc_t *desc, uint8_t i2c_device, uint8_t address)
{
    ASSERT_NOT(desc == NULL);

    desc->i2c_device = i2c_device;
    desc->address = address;

    uint8_t id = readReg(desc, REG_WHO_AM_I);
    if (id != DEVICE_ID) {
        return false;
    }

    writeReg(desc, REG_INTERRUPT_CFG, 0x0); // all interrupts disabled
    writeReg(desc, REG_IF_CTRL, 0x02);      // disable all pull up/downs, disable i3c interface
    writeReg(desc, REG_CTRL1, 0x02);    // block data update, disable low pass, single power down
    writeReg(desc, REG_CTRL2, 0x10);    // interrupt active high, push-pull, autoincrement registers
    writeReg(desc, REG_CTRL3, 0x0);     // data ready signal disable
    writeReg(desc, REG_FIFO_CTRL, 0x0); // bypass mode

    return true;
}
