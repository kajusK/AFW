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
 * @file    drivers/ssd1306.c
 * @brief   SSD1306 oled controller driver, 128x64
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_SSD1306_H
#define __DRIVERS_SSD1306_H

#include <types.h>

/** Screen dimensions */
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

/** Calculate required framebuffer size */
#define SSD1306_FBUF_SIZE ((SSD1306_WIDTH+1)*SSD1306_HEIGHT/8)

/** Default contrast value */
#define SSD1306_INITIAL_CONTRAST 0x7f

/** I2C address for D/C = 0 */
#define SSD1306_ADDR_0 0x3c
/** I2C address for D/C = 1 */
#define SSD1306_ADDR_1 0x3d

/** The device descriptor */
typedef struct {
    uint8_t i2c_device;
    uint8_t address;
    /*
     * Framebuffer for the display data, first byte is ssd1306 control byte to
     * set data transfer to display ram, followed by raw data, 1 byte equals
     * to 8 vertically arranged pixels
     */
    uint8_t *fbuf;      /**< Framebuffer of size SSD1306_FBUF_SIZE */
} ssd1306_desc_t;

/**
 * Draw single pixel to internal frame buffer
 *
 * @param desc  The device descriptor
 * @param x     Horizontal position
 * @param y     Vertical position
 * @param value Value for given pixel
 */
extern void SSD1306_DrawPixel(const ssd1306_desc_t *desc, uint16_t x,
        uint16_t y, bool value);

/**
 * Flush data from internal frame buffer to display
 *
 * @param desc      The device descriptor
 */
extern void SSD1306_Flush(const ssd1306_desc_t *desc);

/**
 * Control display power
 *
 * @param desc      The device descriptor
 * @param on    If true, turn display on, if false, make it sleep
 */
extern void SSD1306_DispEnable(const ssd1306_desc_t *desc, bool on);

/**
 * Set display contrast
 *
 * @param desc      The device descriptor
 * @param contrast  Contrast value, 0-255
 */
extern void SSD1306_SetContrast(const ssd1306_desc_t *desc, uint8_t contrast);

/**
 * Set display orientation
 *
 * @param desc      The device descriptor
 * @param flip      Rotate display by 180 degrees if true
 */
extern void SSD1306_SetOrientation(const ssd1306_desc_t *desc, bool flip);

/**
 * Initialize the display
 *
 * @param [out] desc    The device descriptor
 * @param fbuf          The frame buffer to use with this device
 * @param i2c_device    The I2C device the display is connected to
 * @param address       The I2C address of the device
 * @param reset_port    The MCU port connected to reset pin
 * @param reset_pad     The MCU pin connected to reset pin
 *
 * @return      True if display is responding to commands
 */
extern bool SSD1306_Init(ssd1306_desc_t *desc, uint8_t *fbuf,
        uint8_t i2c_device, uint8_t address,
        uint32_t reset_port, uint8_t reset_pad);

#endif

/** @} */
