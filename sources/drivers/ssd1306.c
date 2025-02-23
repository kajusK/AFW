/**
 * @file    drivers/ssd1306.c
 * @brief   SSD1306 oled controller driver, 128x64
 */

#include <types.h>
#include "hal/i2c.h"
#include "hal/io.h"
#include "utils/time.h"
#include "drivers/ssd1306.h"

typedef enum {
    SSD1306_MEM_MODE = 0x20,          /** modes - page, vertical, horizontal */
    SSD1306_HV_COL_ADDR = 0x21,       /** Min and max col addr in non page mode */
    SSD1306_HV_PAGE_ADDR = 0x22,      /** Min and max page addr in non page mode */
    SSD1306_START_LINE = 0x40,        /** Line to start drawing to disp from */
    SSD1306_CONTRAST = 0x81,          /** Contrast setup */
    SSD1306_DISP_OFF = 0xae,          /** Turn display off */
    SSD1306_DISP_ON = 0xaf,           /** Turn display on */
    SSD1306_MULTIPLEX = 0xa8,         /** Set multiplex - number of lines */
    SSD1306_VERTICAL_NORMAL = 0xc0,   /** Show display non flipped vertically */
    SSD1306_VERTICAL_FLIP = 0xc8,     /** Flip display vertically */
    SSD1306_HORIZONTAL_NORMAL = 0xa0, /** Normal horizontal orientation */
    SSD1306_HORIZONTAL_FLIP = 0xa1,   /** Flip display horizontally */
    SSD1306_CLK_DIV = 0xd5,           /** Set display clock divide ratio */
    SSD1306_PRECHARGE = 0xd9,         /** Set precharge period */
    SSD1306_COM_PINS = 0xda,          /** Set pins layout to match oled hw */
    SSD1306_VCOM_DETECT = 0xdb,       /** Adjust Vcom regulator output */
    SSD1306_CHARGE_PUMP = 0x8d,       /** Configure charge pump */
    SSD1306_DISP_NORM = 0xa6,         /** 1 = black, 0 = white */
    SSD1306_DISP_INVERTED = 0xa7,     /** Invert display colors */
} ssd1306_cmd_t;

/**
 * Send graphical data to display
 *
 * @param buf       Data buffer to be send
 * @param len       Lenght of the buffer
 *
 * @return  True if ACKed
 */
static bool SSD1306i_Data(const ssd1306_desc_t *desc, const uint8_t *buf, size_t len)
{
    return I2Cd_Transceive(desc->i2c_device, desc->address, buf, len, NULL, 0);
}

/**
 * Send command to display
 *
 * @param cmd       Command
 *
 * @return  True if ACKed
 */
static bool SSD1306i_Cmd(const ssd1306_desc_t *desc, ssd1306_cmd_t cmd)
{
    uint8_t buf[2];

    buf[0] = 0x00; /* control byte = 0 -> command */
    buf[1] = cmd;

    return I2Cd_Transceive(desc->i2c_device, desc->address, buf, sizeof(buf), NULL, 0);
}

/**
 * Send command with additional data to display
 *
 * @param cmd       Command
 * @param data      Data for the command
 */
static void SSD1306i_Cmd2(const ssd1306_desc_t *desc, ssd1306_cmd_t cmd, uint8_t data)
{
    /* not a bug, command data are really sent as another command... */
    SSD1306i_Cmd(desc, cmd);
    SSD1306i_Cmd(desc, data);
}

/**
 * Send command with 2 additional data to display
 *
 * @param cmd       Command
 * @param data1     Data for the command
 * @param data2     Data for the command
 */
static void SSD1306i_Cmd3(const ssd1306_desc_t *desc, ssd1306_cmd_t cmd, uint8_t data1,
    uint8_t data2)
{
    SSD1306i_Cmd(desc, cmd);
    SSD1306i_Cmd(desc, data1);
    SSD1306i_Cmd(desc, data2);
}

void SSD1306_DrawPixel(const ssd1306_desc_t *desc, uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t bit = 1 << (y & 7);
    uint16_t pos = x + y / 8 * (SSD1306_WIDTH + 1);
    /* first byte is ssd1306 command */
    pos += 1;

    /* Ignore drawing outside the buffer */
    if (pos >= SSD1306_FBUF_SIZE || x >= SSD1306_WIDTH) {
        return;
    }

    if (color) {
        desc->fbuf[pos] |= bit;
    } else {
        desc->fbuf[pos] &= ~bit;
    }
}

void SSD1306_Flush(const ssd1306_desc_t *desc)
{
    uint8_t page = SSD1306_HEIGHT / 8;
    uint8_t *addr = desc->fbuf;

    /* reset ram address pinter */
    SSD1306i_Cmd(desc, SSD1306_START_LINE | 0);

    /* write split into pages due to i2c driver limitation of max tx data len */
    while (page--) {
        /* first byte in fbuf is control byte 0x40 with data mode */
        SSD1306i_Data(desc, addr, SSD1306_WIDTH + 1);
        addr += SSD1306_WIDTH + 1;
    }
}

void SSD1306_DispEnable(const ssd1306_desc_t *desc, bool on)
{
    if (on) {
        SSD1306i_Cmd(desc, SSD1306_DISP_ON);
    } else {
        SSD1306i_Cmd(desc, SSD1306_DISP_OFF);
    }
}

void SSD1306_SetContrast(const ssd1306_desc_t *desc, uint8_t pct)
{
    SSD1306i_Cmd2(desc, SSD1306_CONTRAST, pct * 255U / 100);
}

void SSD1306_SetOrientation(const ssd1306_desc_t *desc, bool flip)
{
    if (flip) {
        SSD1306i_Cmd(desc, SSD1306_HORIZONTAL_FLIP);
        SSD1306i_Cmd(desc, SSD1306_VERTICAL_FLIP);
    } else {
        SSD1306i_Cmd(desc, SSD1306_HORIZONTAL_NORMAL);
        SSD1306i_Cmd(desc, SSD1306_VERTICAL_NORMAL);
    }
}

bool SSD1306_Init(ssd1306_desc_t *desc, uint8_t *fbuf, uint8_t i2c_device, uint8_t address,
    uint32_t reset_port, uint8_t reset_pad)
{
    uint32_t pos;

    ASSERT_NOT(desc == NULL || fbuf == NULL);

    desc->address = address;
    desc->i2c_device = i2c_device;
    desc->fbuf = fbuf;

    /* Initialize first item in framebuffer - data command for ssd1306 */
    for (pos = 0; pos < SSD1306_FBUF_SIZE; pos += SSD1306_WIDTH + 1) {
        desc->fbuf[pos] = SSD1306_START_LINE;
    }

    if (reset_port != 0xff) {
        IOd_SetLine(reset_port, reset_pad, 0);
        delay_ms(20);
        IOd_SetLine(reset_port, reset_pad, 1);
        delay_ms(20);
    }

    if (!SSD1306i_Cmd(desc, SSD1306_DISP_OFF)) {
        return false;
    }
    SSD1306i_Cmd2(desc, SSD1306_MULTIPLEX, SSD1306_HEIGHT - 1);
    SSD1306i_Cmd(desc, SSD1306_START_LINE | 0);
    SSD1306i_Cmd(desc, SSD1306_HORIZONTAL_NORMAL);
    SSD1306i_Cmd(desc, SSD1306_VERTICAL_NORMAL);

#if SSD1306_HEIGHT == 64
    SSD1306i_Cmd2(desc, SSD1306_COM_PINS, 0x12);
#elif SSD1306_HEIGHT == 32
    SSD1306i_Cmd2(desc, SSD1306_COM_PINS, 0x02);
#else
    SSD1306i_Cmd2(desc, SSD1306_COM_PINS, 0x22);
#endif

    SSD1306i_Cmd2(desc, SSD1306_CONTRAST, SSD1306_INITIAL_CONTRAST);
    SSD1306i_Cmd2(desc, SSD1306_CLK_DIV, 0x80);
    SSD1306i_Cmd2(desc, SSD1306_CHARGE_PUMP, 0x14);
    SSD1306i_Cmd2(desc, SSD1306_PRECHARGE, 0x1f);
    SSD1306i_Cmd2(desc, SSD1306_VCOM_DETECT, 0x10);
    SSD1306i_Cmd(desc, SSD1306_DISP_NORM);
    SSD1306i_Cmd2(desc, SSD1306_MEM_MODE, 0); /* horizontal addressing */
    /* Set area to write */
    SSD1306i_Cmd3(desc, SSD1306_HV_COL_ADDR, 0, SSD1306_WIDTH - 1);
    SSD1306i_Cmd3(desc, SSD1306_HV_PAGE_ADDR, 0, SSD1306_HEIGHT / 8 - 1);
    /* write empty framebuffer to clear possible mess in display ram */
    SSD1306_Flush(desc);

    return true;
}
