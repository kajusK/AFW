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
 * MERCHANTABILITY or FITNEST FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file    drivers/st7565r.c
 * @brief   ST7565 monochrome display controller, e.g. in DOGL128
 *
 * https://www.lcd-module.de/eng/pdf/zubehoer/st7565r.pdf
 *
 * @addtogroup drivers
 * @{
 */

#include "hal/spi.h"
#include "hal/io.h"
#include "utils/assert.h"
#include "utils/time.h"

#include "drivers/st7565r.h"

typedef enum {
    DISP_CMD_SET_LINE = 0x40, /**< Set line to write, lower 6 bits are line */
    DISP_CMD_DISP_OFF = 0xae, /**< Turn the display off */
    DISP_CMD_DISP_ON = 0xaf,  /**< Turn the display on */
    DISP_CMD_SET_PAGE = 0xb0, /**< Set page address (lower nibble) */
    DISP_CMD_SET_COLUMN_MSB = 0x10, /**< Set column MSB (in lower nibble) */
    DISP_CMD_SET_COLUMN_LSB = 0x00, /**< Set column LSB (in lower nibble) */
    DISP_CMD_ADC_NORM = 0xa0,       /**< ADC in normal mode */
    DISP_CMD_ADC_REVERSE = 0xa1,    /**< ADC in reverse mode */
    DISP_CMD_SET_NORMAL = 0xa6,   /**< Display normal mode */
    DISP_CMD_SET_REVERSE = 0xa7,  /**< Display reverse mode */
    DISP_CMD_ALL_ON = 0xa5,       /**< All pixels on */
    DISP_CMD_ALL_ON_OFF = 0xa4,   /**< Normal display usage */
    DISP_CMD_BIAS_1_9 = 0xa2,     /**< LCD voltage bias to 1/9 */
    DISP_CMD_BIAS_1_7 = 0xa3,     /**< LCD voltage bias to 1/7 */
    DISP_CMD_RESET = 0xe2,        /**< Internal reset */
    DISP_CMD_COM_NORM = 0xc0,     /**< Normal COM scanning */
    DISP_CMD_COM_REVERSE = 0xc8,  /**< Reversed COM scanning */
    DISP_CMD_SET_POWER = 0x28,    /**< Power control mode (lower 3 bits) */
    DISP_CMD_SET_REGULATOR = 0x20,/**< Voltage regulator divider (lower 3 bits) */
    DISP_CMD_EL_VOLUME = 0x81,    /**< Output voltage volume register (second byte) */
    DISP_CMD_INDICATOR_OFF = 0xac,/**< Turn off static indicator, second byte sets blinking */
    DISP_CMD_INDICATOR_ON = 0xad, /**< Turn on static indicator, second byte sets blinking */
    DISP_CMD_SET_BOOST = 0xf8,    /**< Set booster ratio in second byte */
} st7565r_cmd_t;

#define cs_set() IOd_SetLine(desc->cs_port, desc->cs_pad, false)
#define cs_unset() IOd_SetLine(desc->cs_port, desc->cs_pad, true)

static void ST7565R_WriteCmd(const st7565r_desc_t *desc, uint8_t cmd)
{
    IOd_SetLine(desc->a0_port, desc->a0_pad, false);
    cs_set();
    SPId_Send(desc->spi_device, &cmd, 1);
    cs_unset();
    IOd_SetLine(desc->a0_port, desc->a0_pad, true);
}

static void ST7565R_WriteCmd2(const st7565r_desc_t *desc, uint8_t cmd,
        uint8_t param)
{
    ST7565R_WriteCmd(desc, cmd);
    ST7565R_WriteCmd(desc, param);
}

void ST7565R_DrawPixel(const st7565r_desc_t *desc, uint16_t x, uint16_t y,
        uint16_t color)
{
    uint8_t bit = 1 << (y & 7);
    uint16_t pos = x+y/8*ST7565R_WIDTH;
    ASSERT_NOT(desc == NULL);

    /* Ignore drawing outside the buffer */
    if (pos >= ST7565R_FBUF_SIZE || x >= ST7565R_WIDTH) {
        return;
    }

    if (color) {
        desc->fbuf[pos] |= bit;
    } else {
        desc->fbuf[pos] &= ~bit;
    }
}

void ST7565R_Flush(const st7565r_desc_t *desc)
{
    ASSERT_NOT(desc == NULL);

    for (uint8_t page = 0; page < 8; page++) {
        ST7565R_WriteCmd(desc, DISP_CMD_SET_PAGE | (page & 0x0f));
        ST7565R_WriteCmd(desc, DISP_CMD_SET_COLUMN_MSB);
        if (desc->flipped) {
            ST7565R_WriteCmd(desc, DISP_CMD_SET_COLUMN_LSB | 0x04);
        } else {
            ST7565R_WriteCmd(desc, DISP_CMD_SET_COLUMN_LSB);
        }

        for (uint8_t i = 0; i <ST7565R_WIDTH; i++) {
            cs_set();
            SPId_Send(desc->spi_device, &desc->fbuf[ST7565R_WIDTH*page+i], 1);
            cs_unset();
        }
    }
}

void ST7565R_DispEnable(const st7565r_desc_t *desc, bool on)
{
    ASSERT_NOT(desc == NULL);
    if (on) {
        ST7565R_WriteCmd(desc, DISP_CMD_SET_POWER | 0x7);
        ST7565R_WriteCmd(desc, DISP_CMD_ALL_ON_OFF);
        ST7565R_WriteCmd(desc, DISP_CMD_DISP_ON);
    } else {
        ST7565R_WriteCmd(desc, DISP_CMD_DISP_OFF);
        ST7565R_WriteCmd(desc, DISP_CMD_ALL_ON);
        ST7565R_WriteCmd(desc, DISP_CMD_SET_POWER);
    }
}

void ST7565R_SetContrast(const st7565r_desc_t *desc, uint8_t pct)
{
    ASSERT_NOT(desc == NULL || pct > 100);
    ST7565R_WriteCmd2(desc, DISP_CMD_EL_VOLUME, ((uint16_t)pct*63U/100) & 0x3f);
}

void ST7565R_SetOrientation(st7565r_desc_t *desc, bool flip)
{
    ASSERT_NOT(desc == NULL);
    if (flip) {
        ST7565R_WriteCmd(desc, DISP_CMD_ADC_NORM);
        ST7565R_WriteCmd(desc, DISP_CMD_COM_REVERSE);
        desc->flipped = true;
    } else {
        ST7565R_WriteCmd(desc, DISP_CMD_ADC_REVERSE);
        ST7565R_WriteCmd(desc, DISP_CMD_COM_NORM);
        desc->flipped = false;
    }
    /* Screen has to be redrawn, else the image will be mirrored */
    ST7565R_Flush(desc);
}

void ST7565R_Init(st7565r_desc_t *desc, uint8_t *fbuf, uint8_t spi_device,
        uint32_t cs_port, uint8_t cs_pad, uint32_t a0_port, uint8_t a0_pad,
        uint32_t reset_port, uint8_t reset_pad)
{
    ASSERT_NOT(desc == NULL);
    desc->fbuf = fbuf;
    desc->spi_device = spi_device;
    desc->cs_port = cs_port;
    desc->cs_pad = cs_pad;
    desc->a0_port = a0_port;
    desc->a0_pad = a0_pad;
    desc->reset_port = reset_port;
    desc->reset_pad = reset_pad;
    desc->flipped = 0;

    IOd_SetLine(reset_port, reset_pad, false);
    delay_ms(1);
    IOd_SetLine(reset_port, reset_pad, true);
    delay_ms(1);

    ST7565R_WriteCmd(desc, DISP_CMD_SET_LINE); /* Start line 0 */
    ST7565R_WriteCmd(desc, DISP_CMD_ADC_REVERSE);
    ST7565R_WriteCmd(desc, DISP_CMD_COM_NORM);
    ST7565R_WriteCmd(desc, DISP_CMD_SET_NORMAL);
    ST7565R_WriteCmd(desc, DISP_CMD_BIAS_1_9);
    ST7565R_WriteCmd(desc, DISP_CMD_SET_POWER | 0x7); /* all on */
    ST7565R_WriteCmd2(desc, DISP_CMD_SET_BOOST, 0x00);
    ST7565R_WriteCmd(desc, DISP_CMD_SET_REGULATOR | 0x7);
    ST7565R_WriteCmd2(desc, DISP_CMD_EL_VOLUME, 0x16);
    ST7565R_WriteCmd2(desc, DISP_CMD_INDICATOR_OFF, 0x00);

    ST7565R_Flush(desc);
    ST7565R_WriteCmd(desc, DISP_CMD_DISP_ON);
}

/** @} */
