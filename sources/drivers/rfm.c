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
 * @file    drivers/rfm.c
 * @brief   Driver for Hope RF RFM95 and similar RF modules
 *
 * @addtogroup drivers
 * @{
 */

#include <hal/spi.h>
#include <hal/io.h>
#include <utils/assert.h>
#include <utils/time.h>

#include "drivers/rfm.h"

#define cs_set() IOd_SetLine(desc->cs_port, desc->cs_pad, 0)
#define cs_unset() IOd_SetLine(desc->cs_port, desc->cs_pad, 1)

/* Amount of usable frequency channels */
#define RFM_CHANNELS 8

/* RFM registers */
#define RFM_REG_FIFO 0x00
#define RFM_REG_MODE 0x01
    #define RFM_MODE_SLEEP 0x00
    #define RFM_MODE_STDBY 0x01
    #define RFM_MODE_TX 0x03
    #define RFM_MODE_LORA 0x80
#define RFM_REG_FR_MSB 0x06
#define RFM_REG_FR_MID 0x07
#define RFM_REG_FR_LSB 0x08
#define RFM_REG_PA 0x09
#define RFM_REG_OCP 0x0b
#define RFM_REG_FIFO_PTR 0x0d
#define RFM_REG_FIFO_TX_BASE 0x0e
#define RFM_REG_FIFO_RX_BASE 0x0f
#define RFM_REG_MODEM_CONF1 0x1d
#define RFM_REG_MODEM_CONF2 0x1e
#define RFM_REG_SYMB_TIMEOUT 0x1f
#define RFM_REG_PREAMBLE_MSB 0x20
#define RFM_REG_PREAMBLE_LSB 0x21
#define RFM_REG_PAYLOAD_LEN 0x22
#define RFM_REG_MODEM_CONF3 0x26
#define RFM_REG_INVERT_IQ 0x33
#define RFM_REG_SYNC_WORD 0x39
#define RFM_REG_INVERT_IQ2 0x3b
#define RFM_REG_DIO_MAP 0x40
#define RFM_REG_VER 0x42
    #define RFM_VER_ID 0x12
#define RFM_REG_PA_DAC 0x4d

static const uint8_t rfmi_lora_region_au[RFM_CHANNELS][3] = {
    {0xE5, 0x33, 0x5A},  /* 916.8 MHz */
    {0xE5, 0x40, 0x26},  /* 917.0 MHz */
    {0xE5, 0x4C, 0xF3},  /* 917.2 MHz */
    {0xE5, 0x59, 0xC0},  /* 917.4 MHz */
    {0xE5, 0x66, 0x8D},  /* 917.6 MHz */
    {0xE5, 0x73, 0x5A},  /* 917.8 MHz */
    {0xE5, 0x80, 0x27},  /* 918.0 MHz */
    {0xE5, 0x8C, 0xF3},  /* 918.2 MHz */
};

static const uint8_t rfmi_lora_region_eu[RFM_CHANNELS][3] = {
    {0xD9, 0x06, 0x8B},  /* 868.1 MHz */
    {0xD9, 0x13, 0x58},  /* 868.3 MHz */
    {0xD9, 0x20, 0x24},  /* 868.5 MHz */
    {0xD8, 0xC6, 0x8B},  /* 867.1 MHz */
    {0xD8, 0xD3, 0x58},  /* 867.3 MHz */
    {0xD8, 0xE0, 0x24},  /* 867.5 MHz */
    {0xD8, 0xEC, 0xF1},  /* 867.7 MHz */
    {0xD8, 0xF9, 0xBE},  /* 867.9 MHz */
};

static const uint8_t rfmi_lora_region_us[RFM_CHANNELS][3] = {
    {0xE1, 0xF9, 0xC0},  /* 903.9 MHz */
    {0xE2, 0x06, 0x8C},  /* 904.1 MHz */
    {0xE2, 0x13, 0x59},  /* 904.3 MHz */
    {0xE2, 0x20, 0x26},  /* 904.5 MHz */
    {0xE2, 0x2C, 0xF3},  /* 904.7 MHz */
    {0xE2, 0x39, 0xC0},  /* 904.9 MHz */
    {0xE2, 0x46, 0x8C},  /* 905.1 MHz */
    {0xE2, 0x53, 0x59},  /* 905.3 MHz */
};

static const uint8_t rfmi_lora_region_as[RFM_CHANNELS][3] = {
    {0xE6, 0xCC, 0xF4},  /* 923.2 MHz */
    {0xE6, 0xD9, 0xC0},  /* 923.4 MHz */
    {0xE6, 0x8C, 0xF3},  /* 922.2 MHz */
    {0xE6, 0x99, 0xC0},  /* 922.4 MHz */
    {0xE6, 0xA6, 0x8D},  /* 922.6 MHz */
    {0xE6, 0xB3, 0x5A},  /* 922.8 MHz */
    {0xE6, 0xC0, 0x27},  /* 923.0 MHz */
    {0xE6, 0x80, 0x27},  /* 922.0 MHz */
};

/**
 * Write value to given register
 *
 * @param reg       Register address
 * @param val       Register value
 */
static void RFMi_WriteReg(const rfm_desc_t *desc, uint8_t reg, uint8_t val)
{
    uint8_t data[2];

    /* Write command */
    data[0] = reg | 0x80;
    data[1] = val;

    cs_set();
    SPId_Send(desc->spi_device, data, 2);
    cs_unset();
}

/**
 * Read register value
 *
 * @param reg   Register address
 * @return Register value
 */
static uint8_t RFMi_ReadReg(const rfm_desc_t *desc, uint8_t reg)
{
    uint8_t data = reg & 0x7f;

    cs_set();
    SPId_Send(desc->spi_device, &data, 1);
    SPId_Receive(desc->spi_device, &data, 1);
    cs_unset();

    return data;
}

/**
 * Write given amount of bytes into FIFO buffer
 *
 * @param reg       Address of the fifo register
 * @param data      Data to be written
 * @param len       Amount of bytes to write
 */
static void RFMi_WriteFifo(const rfm_desc_t *desc, uint8_t reg,
        const uint8_t *data, size_t len)
{
    uint8_t addr = reg | 0x80;

    cs_set();
    SPId_Send(desc->spi_device, &addr, 1);
    SPId_Send(desc->spi_device, data, len);
    cs_unset();
}

void RFM_SetPowerDBm(const rfm_desc_t *desc, int8_t power)
{
    /* doesn't seem to work when PA is disabler - RFO pin not connected? */
    bool paEnabled = true;
    uint8_t maxPower = 0x07;
    uint8_t outPower;

    ASSERT_NOT(power > 20 || power < 2);

    /* Pout = 17 - (15 - outPower) */
    outPower = power - 2;

    /* Over 17, only 20 dBm mode is possible */
    if (power > 17) {
        RFMi_WriteReg(desc, RFM_REG_PA_DAC, 0x87);
        RFMi_WriteReg(desc, RFM_REG_PA, 0xff);
    } else {
        RFMi_WriteReg(desc, RFM_REG_PA_DAC, 0x84);
        RFMi_WriteReg(desc, RFM_REG_PA, (paEnabled << 7) |
                (maxPower << 4) | outPower);
    }
}

void RFM_SetLoraParams(const rfm_desc_t *desc, rfm_bw_t bandwidth, rfm_sf_t sf)
{
    uint8_t reg_bw;
    ASSERT_NOT(sf > 12 || sf < 7);

    switch (bandwidth) {
        case RFM_BW_125k:
            reg_bw = 0x07;
            break;
        case RFM_BW_250k:
            reg_bw = 0x08;
            break;
        case RFM_BW_500k:
            reg_bw = 0x09;
            break;
        default:
            reg_bw = 0x07;
            break;
    }


    /* bandwidth, coding rate 4/5, explicit header mode */
    RFMi_WriteReg(desc, RFM_REG_MODEM_CONF1, (reg_bw << 4) | 0x02);
    /* SF rate, crc-enable */
    RFMi_WriteReg(desc, RFM_REG_MODEM_CONF2, (sf << 4) | 0x04);

    /* Low data rate optimization when symbol length exceeds 16 ms */
    if (sf > 10 && bandwidth == RFM_BW_125k) {
        /* low data rate optimization on, agc on */
        RFMi_WriteReg(desc, RFM_REG_MODEM_CONF3, 0x0c);
    } else {
        /* Agc on */
        RFMi_WriteReg(desc, RFM_REG_MODEM_CONF3, 0x04);
    }
}

void RFM_SetLoraRegion(rfm_desc_t *desc, rfm_lora_region_t region)
{
    switch (region) {
        case RFM_REGION_AU915:
            desc->region = rfmi_lora_region_au;
            break;
        case RFM_REGION_EU863:
            desc->region = rfmi_lora_region_eu;
            break;
        case RFM_REGION_US902:
            desc->region = rfmi_lora_region_us;
            break;
        case RFM_REGION_AS920:
            desc->region = rfmi_lora_region_as;
            break;
        default:
            desc->region = rfmi_lora_region_eu;
            break;
    }
}

void RFM_LoraSend(const rfm_desc_t *desc, const uint8_t *data, size_t len)
{
    uint8_t channel;

    RFMi_WriteReg(desc, RFM_REG_MODE, RFM_MODE_LORA | RFM_MODE_STDBY);
    delay_ms(10);
    /* DIO pin at tx done event */
    RFMi_WriteReg(desc, RFM_REG_DIO_MAP, 0x40);

    /* Random chanel, selection based on last byte of data (last byte of MIC) */
    channel = data[len - 1] % RFM_CHANNELS;
    RFMi_WriteReg(desc, RFM_REG_FR_MSB, desc->region[channel][0]);
    RFMi_WriteReg(desc, RFM_REG_FR_MID, desc->region[channel][1]);
    RFMi_WriteReg(desc, RFM_REG_FR_LSB, desc->region[channel][2]);

    RFMi_WriteReg(desc, RFM_REG_PAYLOAD_LEN, len);
    RFMi_WriteReg(desc, RFM_REG_FIFO_PTR, 0x80);
    RFMi_WriteFifo(desc, RFM_REG_FIFO, data, len);
    RFMi_WriteReg(desc, RFM_REG_MODE, RFM_MODE_LORA | RFM_MODE_TX);

    /* Wait for data to be transmitted */
    while (IOd_GetLine(desc->io0_port, desc->io0_pad) != 0) {
        ;
    }
    //hotfix TODO
    delay_ms(550);

    RFMi_WriteReg(desc, RFM_REG_MODE, RFM_MODE_LORA | RFM_MODE_SLEEP);
}

bool RFM_LoraInit(rfm_desc_t *desc, uint8_t spi_device, uint32_t cs_port,
        uint8_t cs_pad, uint32_t reset_port, uint8_t reset_pad,
        uint32_t io0_port, uint8_t io0_pad)
{
    ASSERT_NOT(desc == NULL);
    desc->spi_device = spi_device;
    desc->cs_port = cs_port;
    desc->cs_pad = cs_pad;
    desc->reset_port = reset_port;
    desc->reset_pad = reset_pad;
    desc->io0_port = io0_port;
    desc->io0_port = io0_pad;

    /* Reset the device */
    IOd_SetLine(desc->reset_port, desc->reset_pad, 0);
    delay_ms(1);
    IOd_SetLine(desc->reset_port, desc->reset_pad, 1);
    delay_ms(5);

    /* Verify device signature */
    if (RFMi_ReadReg(desc, RFM_REG_VER) != RFM_VER_ID) {
        return false;
    }

    /* Mode can be changed only in sleep */
    RFMi_WriteReg(desc, RFM_REG_MODE, RFM_MODE_SLEEP);
    RFMi_WriteReg(desc, RFM_REG_MODE, RFM_MODE_LORA);

    /* Set over current protection to 240 mA */
    RFMi_WriteReg(desc, RFM_REG_OCP, 0x1f);
    /* Rx timeout is 37 symbols */
    RFMi_WriteReg(desc, RFM_REG_SYMB_TIMEOUT, 0x25);
    /* Preamble length is 8 symbols */
    RFMi_WriteReg(desc, RFM_REG_PREAMBLE_MSB, 0x00);
    RFMi_WriteReg(desc, RFM_REG_PREAMBLE_LSB, 0x08);
    /* AGC auto, low data rate optimizations off */
    RFMi_WriteReg(desc, RFM_REG_MODEM_CONF3, 0x0c);
    /* LORA sync word */
    RFMi_WriteReg(desc, RFM_REG_SYNC_WORD, 0x34);
    /* IQ config */
    RFMi_WriteReg(desc, RFM_REG_INVERT_IQ, 0x27);
    RFMi_WriteReg(desc, RFM_REG_INVERT_IQ2, 0x1d);

    /* FIFO pointers */
    RFMi_WriteReg(desc, RFM_REG_FIFO_TX_BASE, 0x80);
    RFMi_WriteReg(desc, RFM_REG_FIFO_RX_BASE, 0x00);

    /* Default lora parameters */
    RFM_SetLoraParams(desc, RFM_BW_125k, RFM_SF_7);
    RFM_SetPowerDBm(desc, 17);
    RFM_SetLoraRegion(desc, RFM_REGION_EU863);

    return true;
}

/** @} */
