/**
 * @file    drivers/st7565r.h
 * @brief   ST7565 monochrome display controller, e.g. in DOGL128
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
void ST7565R_DrawPixel(const st7565r_desc_t *desc, uint16_t x, uint16_t y,
        uint16_t color);

/**
 * Flush data from internal frame buffer to display
 *
 * @param desc      The device descriptor
 */
void ST7565R_Flush(const st7565r_desc_t *desc);

/**
 * Control display power
 *
 * @param desc      The device descriptor
 * @param on        If true, turn display on, if false, make it sleep
 */
void ST7565R_DispEnable(const st7565r_desc_t *desc, bool on);

/**
 * Set display contrast
 *
 * @param desc      The device descriptor
 * @param pct       Contrast value in percent
 */
void ST7565R_SetContrast(const st7565r_desc_t *desc, uint8_t pct);

/**
 * Set display orientation
 *
 * @param desc      The device descriptor
 * @param flip      Rotate display by 180 degrees if true
 */
void ST7565R_SetOrientation(st7565r_desc_t *desc, bool flip);

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
void ST7565R_Init(st7565r_desc_t *desc, uint8_t *fbuf,
        uint8_t spi_device, uint32_t cs_port, uint8_t cs_pad, uint32_t a0_port,
        uint8_t a0_pad, uint32_t reset_port, uint8_t reset_pad);

#endif
