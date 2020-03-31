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
 * @file    gui.h
 * @brief   Small custom GUI library supporting basic drawing on graphical lcd
 *
 * @addtogroup cgui
 * @{
 */

#ifndef __CGUI_CGUI_H
#define __CGUI_CGUI_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

typedef void (*cgui_draw_pixel_t)(uint16_t x, uint16_t y, bool val);
typedef void (*cgui_fill_frame_t)(uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2, bool val);
typedef void (*cgui_draw_line)(uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2);

typedef struct {
    const uint8_t *img;     /** Array of image bits (1 bit per pixel) */
    uint16_t width;
    uint16_t height;
    bool transparent;       /** set to true to draw only white pixels */
} cgui_img_t;

typedef struct {
    const uint8_t *chars;   /** Array of characters, each width*height/8 */
    uint8_t width;
    uint8_t height;
    uint8_t start_id;
    uint8_t end_id;
} cgui_font_t;

typedef struct {
    cgui_draw_pixel_t draw;     /** Draw pixel to given location */
    cgui_fill_frame_t fill;     /** Optional (NULL), fill box with color */
    cgui_draw_line line;        /** Optional (NULL), draw line */
    uint16_t width;             /** Screen width in px */
    uint16_t height;            /** Screen height in px */
    const cgui_font_t *font;    /** Currently used font */
} cgui_display_t;

#include "fonts.h"

/**
 * Fill whole screen with selected value
 *
 * @param value     Pixel value to fill screen with
 */
extern void Cgui_FillScreen(bool value);

/**
 * Draw single pixel
 *
 * @param x     Horizontal position
 * @param y     Vertical position
 * @param value Pixel value
 */
extern void Cgui_DrawPixel(uint16_t x, uint16_t y, bool value);

/**
 * Draw line one pixel thick
 *
 * @param x1    Horizontal starting coordinate
 * @param y1    Vertical starting coordinate
 * @param x2    Horizontal ending coordinate
 * @param y2    Vertical ending coordinate
 */
extern void Cgui_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * Draw rectangular box
 *
 * @param x1    First corner x coordinate
 * @param y1    First corner y coordinate
 * @param x2    Second corner x coordinate
 * @param y2    Second corner y coordinate
 */
extern void Cgui_DrawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * Draw rectangular filled box
 *
 * @param x1    First corner x coordinate
 * @param y1    First corner y coordinate
 * @param x2    Second corner x coordinate
 * @param y2    Second corner y coordinate
 * @param value Value to fill box with
 */
extern void Cgui_DrawFilledBox(uint16_t x1, uint16_t y1, uint16_t x2,
        uint16_t y2, bool value);

/**
 * Draw rectangular filled box
 *
 * @param pos_x    Upper left corner
 * @param pox_y    Upper left corner
 * @param img      Image to be drawn
 */
extern void Cgui_DrawImage(uint16_t pos_x, uint16_t pos_y, cgui_img_t *img);

/**
 * Draw single character on given coordinate (upper left corner)
 *
 * @param x     Horizontal position
 * @param y     Vertical position
 * @param c     Character to draw
 *
 * @return      False if required character not present in font (printed as space)
 */
extern bool Cgui_Putc(uint16_t x, uint16_t y, char c);

/**
 * Draw string
 *
 * @param x     Upper left corner of string beginning
 * @param y     Upper left corner of string beginning
 * @param msg   String to be printed
 */
extern void Cgui_Puts(uint16_t x, uint16_t y, const char *msg);

/**
 * Printf like function (newline supported)
 *
 * @param px    Upper left corner of string beginning
 * @param py    Upper left corner of string beginning
 * @param fmt   String formatter in printf format, only %c,d,s supported
 */
extern void Cgui_Printf(uint16_t px, uint16_t py, const char *fmt, ...);

/**
 * Set font to be used
 *
 * @param font      Font data
 */
extern void Cgui_SetFont(const cgui_font_t *font);

/**
 * Get height of currently used font
 *
 * @return height in px
 */
extern uint16_t Cgui_GetFontHeight(void);

/**
 * Get width of currently used font
 *
 * @return width in px
 */
extern uint16_t Cgui_GetFontWidth(void);

/**
 * Get width of the screen
 *
 * @return width in px
 */
extern uint16_t Cgui_GetWidth(void);

/**
 * Get height of the screen
 *
 * @return height in px
 */
extern uint16_t Cgui_GetHeight(void);

/**
 * Initialize cgui
 *
 * @param draw      Pointer to function to draw pixel on screen
 * @param width     Width of the screen in px
 * @param height    Height of the screen in px
 */
extern void Cgui_Init(cgui_draw_pixel_t draw, uint16_t width,
        uint16_t height);

/**
 * Set hw acceleration support
 *
 * Overrides internal drawing functions by custom (e.g. lcd driver api)
 *
 * @param fill      Pointer to function to fill area of screen (or NULL)
 * @param line      Pointer to function to draw line (or NULL)
 *
 */
extern void Cgui_SetHwAccel(cgui_fill_frame_t fill, cgui_draw_line line);

#endif

/** @} */
