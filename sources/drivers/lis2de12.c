/**
 * @file    drivers/li2de12.c
 * @brief   Driver for ST LIS2DE12 accelerometer
 * @note    Will most likely work for other LIS2* accelerometers, not tested
 */

#include <types.h>
#include <hal/i2c.h>
#include "lis2de12.h"

/* Register map */
#define STATUS_AUX      0x07
#define OUT_TEMP_L      0x0C
#define OUT_TEMP_H      0x0D
#define WHO_AM_I_REG    0x0F
#define CTRL_REG0       0x1E
#define TEMP_CFG_REG    0x1F
#define CTRL_REG1       0x20
#define CTRL_REG2       0x21
#define CTRL_REG3       0x22
#define CTRL_REG4       0x23
#define CTRL_REG5       0x24
#define CTRL_REG6       0x25
#define REFERENCE       0x26
#define STATUS_REG      0x27
#define FIFO_READ_START 0x28
#define OUT_X_H         0x29
#define OUT_Y_H         0x2B
#define OUT_Z_H         0x2D
#define FIFO_CTRL_REG   0x2E
#define FIFO_SRC_REG    0x2F
#define INT1_CFG        0x30
#define INT1_SRC        0x31
#define INT1_THS        0x32
#define INT1_DURATION   0x33
#define INT2_CFG        0x34
#define INT2_SRC        0x35
#define INT2_THS        0x36
#define INT2_DURATION   0x37
#define CLICK_CFG       0x38
#define CLICK_SRC       0x39
#define CLICK_THS       0x3A
#define TIME_LIMIT      0x3B
#define TIME_LATENCY    0x3C
#define TIME_WINDOW     0x3D
#define ACT_TSH         0x3E
#define ACT_DUR         0x3F

/* Register fields*/
#define STATUS_DATA_RDY 0x08u
#define DEVICE_ID       0x33u

static uint8_t readReg(const lis2de12_desc_t *desc, uint8_t addr)
{
    uint8_t value;
    I2Cd_Transceive(desc->i2c_device, desc->address, &addr, 1, &value, 1);
    return value;
}

static void writeReg(const lis2de12_desc_t *desc, uint8_t addr, uint8_t value)
{
    uint8_t buf[2];
    buf[0] = addr;
    buf[1] = value;
    I2Cd_Transceive(desc->i2c_device, desc->address, buf, sizeof(buf), NULL, 0);
}

/**
 * Convert acceleration reading to milli G
 *
 * @param raw   Measured value
 * @param scale Measurement scale
 * @return Value in milli G
 */
static int16_t rawToMg(int8_t raw, lis2de12_scale_t scale)
{
    /* Sensitivity for each measuring range, in 1/10 of of mg/digit unit */
    static const uint16_t sensitivity[] = { 156, 312, 625, 1875 };
    return ((int32_t)raw * sensitivity[scale]) / 10;
}

bool LIS2DE12_GetAccel(const lis2de12_desc_t *desc, int16_t *x_mg, int16_t *y_mg, int16_t *z_mg)
{
    if ((readReg(desc, STATUS_REG) & STATUS_DATA_RDY) == 0) {
        return false;
    }

    if (x_mg != NULL) {
        *x_mg = rawToMg((int8_t)readReg(desc, OUT_X_H), desc->scale);
    }
    if (y_mg != NULL) {
        *y_mg = rawToMg((int8_t)readReg(desc, OUT_Y_H), desc->scale);
    }
    if (z_mg != NULL) {
        *z_mg = rawToMg((int8_t)readReg(desc, OUT_Z_H), desc->scale);
    }
    return true;
}

void LIS2DE12_PowerOn(const lis2de12_desc_t *desc)
{
    writeReg(desc, CTRL_REG1, 0x0f | (desc->odr << 4));
}

void LIS2DE12_PowerOff(const lis2de12_desc_t *desc)
{
    writeReg(desc, CTRL_REG1, 0x80);
}

void LIS2DE12_Configure(lis2de12_desc_t *desc, lis2de12_odr_t odr, lis2de12_scale_t scale)
{
    desc->odr = odr;
    desc->scale = scale;

    if (readReg(desc, CTRL_REG1) & 0xf0) {
        // device is not powered off, update the ODR
        writeReg(desc, CTRL_REG1, 0x0f | (desc->odr << 4));
    }
    writeReg(desc, CTRL_REG4, 0x80 | (desc->scale << 4));
}

bool LIS2DE12_GetClearIntFlag(const lis2de12_desc_t *desc)
{
    // interrupts are cleared by reading INT1_SRC
    return readReg(desc, INT1_SRC) != 0;
}

void LIS2DE12_DisableInt(const lis2de12_desc_t *desc)
{
    writeReg(desc, INT1_CFG, 0x0);
    writeReg(desc, CTRL_REG3, 0x0);
}

void LIS2DE12_EnableInt(const lis2de12_desc_t *desc, uint16_t threshold_mg)
{
    static const uint16_t sensitivity[] = { 16, 32, 62, 186 };
    uint8_t threshold = threshold_mg / sensitivity[desc->scale];

    // upper bit is always 0
    if (threshold > 0x7f) {
        threshold = 0x7f;
    }

    writeReg(desc, INT1_THS, threshold_mg);
    // 1/ODR minimum event duration
    writeReg(desc, INT1_DURATION, 0x01);
    // enable interrupt on all axes for high acceleration
    writeReg(desc, INT1_CFG, 0x2A);
    // clear interrupt flags
    readReg(desc, INT1_SRC);
    // enable IA1 interrupt on INT1 pin
    writeReg(desc, CTRL_REG3, 0x40);
}

bool LIS2DE12_Init(lis2de12_desc_t *desc, uint8_t i2c_device, uint8_t address)
{
    ASSERT_NOT(desc == NULL);

    desc->i2c_device = i2c_device;
    desc->address = address;

    uint8_t id = readReg(desc, WHO_AM_I_REG);
    if (id != DEVICE_ID) {
        return false;
    }
    // Disconnect internal pullups on SDO/SA0
    writeReg(desc, CTRL_REG0, 0x90);
    // Disable temperature sensor
    writeReg(desc, TEMP_CFG_REG, 0x0);
    // Power off
    writeReg(desc, CTRL_REG1, 0x08);
    // Enable high pass filter, normal mode, cutoff ODR/200
    writeReg(desc, CTRL_REG2, 0xA3);
    // Disable interrupts
    writeReg(desc, CTRL_REG3, 0x0);
    // Low power mode, scale +-8g, block data update
    writeReg(desc, CTRL_REG4, 0xA0);
    // Latch INT1, disable FIFO
    writeReg(desc, CTRL_REG5, 0x08);
    // INT1 is active high
    writeReg(desc, CTRL_REG6, 0x0);
    LIS2DE12_Configure(desc, LIS2DE12_ODR_25HZ, LIS2DE12_SCALE_4G);
    return true;
}
