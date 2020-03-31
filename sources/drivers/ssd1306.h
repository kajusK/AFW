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

#define SSD1306_I2C_DEV 1

/** Screen dimensions */
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

/** Default contrast value */
#define SSD1306_INITIAL_CONTRAST 0x7f

/**
 * Draw single pixel to internal frame buffer
 *
 * @param x     Horizontal position
 * @param y     Vertical position
 * @param value Value for given pixel
 */
extern void SSD1306_DrawPixel(uint16_t x, uint16_t y, bool value);

/**
 * Flush data from internal frame buffer to display
 */
extern void SSD1306_Flush(void);

/**
 * Control display power
 *
 * @param on    If true, turn display on, if false, make it sleep
 */
extern void SSD1306_DispEnable(bool on);

/**
 * Set display contrast
 *
 * @param contrast  Contrast value, 0-255
 */
extern void SSD1306_SetContrast(uint8_t contrast);

/**
 * Set display orientation
 *
 * @param flip      Rotate display by 180 degrees if true
 */
extern void SSD1306_SetOrientation(bool flip);

/**
 * Initialize the display
 *
 * @return      True if display is responding to commands
 */
extern bool SSD1306_Init(void);

#endif

/** @} */
