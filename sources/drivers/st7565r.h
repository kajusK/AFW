/*
 * Copyright (C) 2019 Jakub Kaderka
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
 * @file    drivers/st7565r.h
 * @brief   ST7565 monochrome display controller, e.g. in DOGL128
 *
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_SD7565R_H
#define __DRIVERS_SD7565R_H

#include <types.h>

#define ST7565R_WIDTH 128
#define ST7565R_HEIGHT 64

#define ST7565R_FBUF_SIZE (ST7565R_WIDTH*ST7565R_HEIGHT/8)

typedef struct {
    uint32_t cs_port;
    uint32_t a0_port;
    uint32_t reset_port;
    uint8_t spi_device;     /**< Number of SPI device to use */
    uint8_t cs_pad;
    uint8_t a0_pad;
    uint8_t reset_pad;
    uint8_t *fbuf;          /**< Framebuffer of ST7565R_FBUF_SIZE size */
    bool flipped;           /**< Is the display in vertically flipped mode */
} st7565r_desc_t;

/**
 * Draw single pixel to internal frame buffer
 *
 * @param desc  The device descriptor
 * @param x     Horizontal position
 * @param y     Vertical position
 * @param color Value for given pixel (0 = white, others = black)
 */
extern void ST7565R_DrawPixel(const st7565r_desc_t *desc, uint16_t x, uint16_t y,
        uint16_t color);

/**
 * Flush data from internal frame buffer to display
 *
 * @param desc      The device descriptor
 */
extern void ST7565R_Flush(const st7565r_desc_t *desc);

/**
 * Control display power
 *
 * @param desc      The device descriptor
 * @param on        If true, turn display on, if false, make it sleep
 */
extern void ST7565R_DispEnable(const st7565r_desc_t *desc, bool on);

/**
 * Set display contrast
 *
 * @param desc      The device descriptor
 * @param pct       Contrast value in percent
 */
extern void ST7565R_SetContrast(const st7565r_desc_t *desc, uint8_t pct);

/**
 * Set display orientation
 *
 * @param desc      The device descriptor
 * @param flip      Rotate display by 180 degrees if true
 */
extern void ST7565R_SetOrientation(st7565r_desc_t *desc, bool flip);

/**
 * Initialize the display
 *
 * @param [out] desc    The device descriptor
 * @param fbuf          The frame buffer to use with this device
 * @param spi_device    The I2C device the display is connected to
 * @param cs_port       The MCU port connected to cs pin
 * @param cs_pad        The MCU pin connected to cs pin
 * @param a0_port       The MCU port connected to a0 pin
 * @param a0_pad        The MCU pin connected to a0 pin
 * @param reset_port    The MCU port connected to reset pin
 * @param reset_pad     The MCU pin connected to reset pin
 *
 * @return      True if display is responding to commands
 */
extern void ST7565R_Init(st7565r_desc_t *desc, uint8_t *fbuf,
        uint8_t spi_device, uint32_t cs_port, uint8_t cs_pad, uint32_t a0_port,
        uint8_t a0_pad, uint32_t reset_port, uint8_t reset_pad);

#endif

/** @} */
