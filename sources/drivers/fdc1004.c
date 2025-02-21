/**
 * @file    drivers/fdc1004.c
 * @brief   Driver for FDC1004 capacitance to digital converter
 *
 * https://www.ti.com/lit/ds/symlink/fdc1004.pdf
 */

#include <types.h>
#include <hal/i2c.h>
#include "drivers/fdc1004.h"

/** I2C address of the device */
#define FDC1004_ADDR 0x50

/** Device registers */
typedef enum {
    FDC_MEAS1_MSB = 0x00,
    FDC_MEAS1_LSB = 0x01,
    FDC_MEAS2_MSB = 0x02,
    FDC_MEAS2_LSB = 0x03,
    FDC_MEAS3_MSB = 0x04,
    FDC_MEAS3_LSB = 0x05,
    FDC_MEAS4_MSB = 0x06,
    FDC_MEAS4_LSB = 0x07,
    FDC_CONF_MEAS1 = 0x08,
    FDC_CONF_MEAS2 = 0x09,
    FDC_CONF_MEAS3 = 0x0A,
    FDC_CONF_MEAS4 = 0x0B,
    FDC_CONF = 0x0C,
    FDC_OFFSET_CIN1 = 0x0D,
    FDC_OFFSET_CIN2 = 0x0E,
    FDC_OFFSET_CIN3 = 0x0F,
    FDC_OFFSET_CIN4 = 0x10,
    FDC_GAIN_CIN1 = 0x11,
    FDC_GAIN_CIN2 = 0x12,
    FDC_GAIN_CIN3 = 0x13,
    FDC_GAIN_CIN4 = 0x14,
    FDC_MANUFACTURER_ID = 0xFE,
    FDC_DEVICE_ID = 0xFF
} fdc1004_reg_t;

/**
 * Write to FDC1004 register
 *
 * @param desc  The device descriptor
 * @param reg   Register address
 * @param data  Data buffer to write
 * @return True if sensor acked the command
 */
static bool FDC1004i_WriteReg(const fdc1004_desc_t *desc, fdc1004_reg_t reg,
        uint16_t data)
{
    uint8_t buf[3];

    ASSERT_NOT(desc == NULL);
    buf[0] = reg;
    buf[1] = (data >> 8) & 0xff;
    buf[2] = data & 0xff;

    return I2Cd_Transceive(desc->i2c_device, FDC1004_ADDR, buf, sizeof(buf), NULL, 0);
}

/**
 * Read from FDC1004 register
 *
 * @param desc  The device descriptor
 * @param reg   Register address
 * @param data  Data buffer to read to
 * @return True if sensor acked the command
 */
static bool FDC1004i_ReadReg(const fdc1004_desc_t *desc, fdc1004_reg_t reg,
        uint16_t *data)
{
    uint8_t buf[2];
    ASSERT_NOT(desc == NULL || data == NULL);

    if (!I2Cd_Transceive(desc->i2c_device, FDC1004_ADDR, &reg, 1, buf, 2)) {
        return false;
    }
    *data = (buf[0] << 8) | buf[1];
    return true;
}

bool FDC1004_ReadResultRaw(const fdc1004_desc_t *desc,
        fdc1004_meas_t channel, uint32_t *raw)
{
    uint16_t msb, lsb;
    bool ret = false;

    ASSERT_NOT(raw == NULL);

    switch (channel) {
        case FDC_MEAS_1:
            ret = FDC1004i_ReadReg(desc, FDC_MEAS1_MSB, &msb);
            ret &= FDC1004i_ReadReg(desc, FDC_MEAS1_LSB, &lsb);
            break;
        case FDC_MEAS_2:
            ret = FDC1004i_ReadReg(desc, FDC_MEAS2_MSB, &msb);
            ret &= FDC1004i_ReadReg(desc, FDC_MEAS2_LSB, &lsb);
            break;
        case FDC_MEAS_3:
            ret = FDC1004i_ReadReg(desc, FDC_MEAS3_MSB, &msb);
            ret &= FDC1004i_ReadReg(desc, FDC_MEAS3_LSB, &lsb);
            break;
        case FDC_MEAS_4:
            ret = FDC1004i_ReadReg(desc, FDC_MEAS4_MSB, &msb);
            ret &= FDC1004i_ReadReg(desc, FDC_MEAS4_LSB, &lsb);
            break;
    }
    if (!ret) {
        return false;
    }

    /* LSB lower 8 bits are always zero */
    *raw = (msb << 8) | (lsb >> 8);
    return true;
}

bool FDC1004_IsMeasComplete(const fdc1004_desc_t *desc, fdc1004_meas_t channel)
{
    uint16_t data;

    if (!FDC1004i_ReadReg(desc, FDC_CONF, &data)) {
        return false;
    }

    switch (channel) {
        case FDC_MEAS_1:
            return data & 0x08;
        case FDC_MEAS_2:
            return data & 0x04;
        case FDC_MEAS_3:
            return data & 0x02;
        case FDC_MEAS_4:
            return data & 0x01;
    }
    return false;
}

bool FDC1004_ConfigureMeasurement(const fdc1004_desc_t *desc,
        fdc1004_meas_t channel, fdc1004_ch_t positive, fdc1004_ch_t negative,
        uint32_t offset_pf)
{
    uint16_t conf;
    uint8_t offset;

    ASSERT(positive <= FDC_CIN4 && positive < negative);

    offset = (offset_pf * 1000) / 3125;
    if (offset > 0x1f) {
        offset = 0x1f;
    }
    conf = positive << 13 | negative << 10 | offset << 5;

    switch (channel) {
        case FDC_MEAS_1:
            return FDC1004i_WriteReg(desc, FDC_CONF_MEAS1, conf);
        case FDC_MEAS_2:
            return FDC1004i_WriteReg(desc, FDC_CONF_MEAS2, conf);
        case FDC_MEAS_3:
            return FDC1004i_WriteReg(desc, FDC_CONF_MEAS3, conf);
        case FDC_MEAS_4:
            return FDC1004i_WriteReg(desc, FDC_CONF_MEAS4, conf);
    }
    return false;
}

bool FDC1004_RunSingle(const fdc1004_desc_t *desc, fdc1004_rate_t rate,
        fdc1004_meas_t channel)
{
    switch (channel) {
        case FDC_MEAS_1:
            return FDC1004i_WriteReg(desc, FDC_CONF, rate << 10 | 1 << 7);
        case FDC_MEAS_2:
            return FDC1004i_WriteReg(desc, FDC_CONF, rate << 10 | 1 << 6);
        case FDC_MEAS_3:
            return FDC1004i_WriteReg(desc, FDC_CONF, rate << 10 | 1 << 5);
        case FDC_MEAS_4:
            return FDC1004i_WriteReg(desc, FDC_CONF, rate << 10 | 1 << 4);
    }
    return false;
}

bool FDC1004_RunRepeated(const fdc1004_desc_t *desc, fdc1004_rate_t rate,
        uint8_t channels)
{
    uint16_t data = rate << 10 | 1 << 8;

    if (channels & FDC_MEAS_1) { data |= 1 << 7; }
    if (channels & FDC_MEAS_2) { data |= 1 << 6; }
    if (channels & FDC_MEAS_3) { data |= 1 << 5; }
    if (channels & FDC_MEAS_4) { data |= 1 << 4; }

    return FDC1004i_WriteReg(desc, FDC_CONF, data);
}

bool FDC1004_Init(fdc1004_desc_t *desc, uint8_t i2c_device)
{
    uint16_t data = 1 << 15;
    uint16_t manufacturer;
    bool ret;
    ASSERT_NOT(desc == NULL);
    desc->i2c_device = i2c_device;

    /* Factory reset the device */
    FDC1004i_WriteReg(desc, FDC_CONF, data);

    ret = FDC1004i_ReadReg(desc, FDC_MANUFACTURER_ID, &manufacturer);
    if (!ret || manufacturer != 0x5449) {
        return false;
    }
    return true;
}
