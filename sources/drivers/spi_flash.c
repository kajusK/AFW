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
 * @file    drivers/spi_flash.h
 * @brief   Driver for SST26 SPI flash memories
 *
 * @addtogroup drivers
 * @{
 */

#include "utils/assert.h"
#include "utils/time.h"
#include "hal/io.h"
#include "hal/spi.h"

#include "drivers/spi_flash.h"

#define SPI_DEVICE 1
#define PAGE_BYTES 256
#define CHIP_ERASE_TIME_MS 40
#define PAGE_ERASE_TIME_MS 20
#define WRITE_PAGE_TIME_MS 2

/** status register */
enum {
    STATUS_SEC = 0x04,  /** Security ID status, 1 = locked */
    STATUS_WPLD = 0x08, /** Write protection lock down, 1 = enabled */
    STATUS_WSP = 0x10,  /** Write suspend-program status, 1 = suspended */
    STATUS_WSE = 0x20,  /** Write suspend-erase status, 1 = suspended */
    STATUS_WEL = 0x40,  /** Write enable latch status, 1 = write-enabled */
    STATUS_BUSY = 0x80, /** Write operation status, 1 = write in progress */
};

/** Flash memory commands */
typedef enum {
    /* Configuration */
    CMD_NOP = 0x00,
    CMD_RSTEN = 0x66,   /** reset enable */
    CMD_RST = 0x99,     /** reset memory */
    CMD_EQIO = 0x38,    /** enable quad io */
    CMD_RSTQIO = 0xff,  /** reset quad io */
    CMD_RDSR = 0x05,    /** read status reg */

    /* read */
    CMD_READ = 0x03,    /** read memory */
    CMD_HSREAD = 0x0b,  /** High speed read */
    CMD_SB = 0xc0,      /** set burst length */
    CMD_RBSQI = 0x0c,   /** SQI read burst with wrap */

    /* Identification */
    CMD_JEDEC = 0x9f,   /** read jedec id */
    CMD_QJID = 0xaf,    /** read quad IO J-ID */

    /* write */
    CMD_WREN = 0x06,    /** write enable */
    CMD_WRDI = 0x04,    /** write_disable */
    CMD_SE = 0x20,      /** Erase 4 kB */
    CMD_BE = 0xd8,      /** Erase 64,32 or 8 k memory */
    CMD_CE = 0xc7,      /** Erase full array */
    CMD_PP = 0x02,      /** Page program */
    CMD_WRSU = 0xb0,    /** Suspend program/erase */
    CMD_WRRE = 0x30,    /** Resume program/erase */

    /* protection */
    CMD_RBPR = 0x72,    /** Read block protection reg */
    CMD_WBPR = 0x42,    /** Write block protection reg */
    CMD_LBPR = 0x8d,    /** Lock down block protection reg */
    CMD_RSID = 0x88,    /** Read security ID */
    CMD_PSID = 0xa5,    /** Program user security ID */
    CMD_LSID = 0x85,    /** Lock out security Id programming */
} spiflash_cmd_t;

/**
 * Drive cs pin to selected level
 *
 * @param val       true for low (selected), false for high
 */
static void SpiFlashi_Cs(bool val)
{
    IOd_SetLine(LINE_FLASH_CS, !val);
}

/**
 * Send command followed by address and optionally also dummy bytes
 *
 * @param cmd       Command
 * @param addr      24 bytes of address
 * @param dummy     Amount (up to 4) of dummy bytes to send after address
 * @param release_cs    Release cs pin after sending data
 */
static void SpiFlashi_CmdWithAddr(spiflash_cmd_t cmd, uint32_t addr,
        uint8_t dummy, bool release_cs)
{
    uint8_t data[8] = {0};

    ASSERT_NOT(dummy > 4);

    data[0] = cmd;
    data[1] = addr >> 16;
    data[2] = addr >> 8;
    data[3] = addr;

    SpiFlashi_Cs(true);
    SPId_Send(SPI_DEVICE, data, 4+dummy);

    if (release_cs) {
        SpiFlashi_Cs(false);
    }
}

/**
 * Send Flash command without any additional parameters
 *
 * @param cmd   Command to be executed
 */
static void SpiFlashi_Cmd(spiflash_cmd_t cmd)
{
    SpiFlashi_Cs(true);
    SPId_Send(SPI_DEVICE, &cmd, 1);
    SpiFlashi_Cs(false);
}

/**
 * Wait for flash operation to finish
 *
 * @param timeout_ms    Time to wait for op to finish
 */
static void SpiFlashi_WaitReady(uint32_t timeout_ms)
{
    const uint8_t cmd = CMD_RDSR;
    uint32_t start = millis();

    SpiFlashi_Cs(true);
    SPId_Send(SPI_DEVICE, &cmd, 1);

    while ((millis() - start) < timeout_ms) {
        /* continuously read status register, command send only once */
        if ((SPId_Transceive(SPI_DEVICE, 0xff) & STATUS_BUSY) == 0) {
            break;
        }
    }

    SpiFlashi_Cs(false);
}

/**
 * Enable write protection
 */
static void SpiFlashi_WriteEnable(void)
{
    SpiFlashi_Cmd(CMD_WREN);
}

/**
 * Disable write protection
 */
static void SpiFlashi_WriteDisable(void)
{
    SpiFlashi_Cmd(CMD_WRDI);
}

void SpiFlash_WriteUnlock(void)
{
    uint8_t buf[11] = {0};
    buf[0] = CMD_WBPR;

    SpiFlashi_WriteEnable();
    SpiFlashi_Cs(true);
    SPId_Send(SPI_DEVICE, buf, sizeof(buf));
    SpiFlashi_Cs(false);
    SpiFlashi_WriteDisable();
}

void SpiFlash_Read(uint32_t addr, uint8_t *buf, size_t len)
{
    SpiFlashi_CmdWithAddr(CMD_READ, addr, 0, false);
    SPId_Receive(SPI_DEVICE, buf, len);
    SpiFlashi_Cs(false);
}

void SpiFlash_Write(uint32_t addr, const uint8_t *buf, size_t len)
{
    uint16_t bytes;
    size_t offset = 0;

    /* up to 256 bytes in single write */
    while (len != 0) {
        if (len >= PAGE_BYTES) {
            bytes = PAGE_BYTES;
        } else {
            bytes = len;
        }
        SpiFlashi_WriteEnable();
        SpiFlashi_CmdWithAddr(CMD_PP, addr, 0, false);
        SPId_Send(SPI_DEVICE, buf, bytes);
        SpiFlashi_Cs(false);
        SpiFlashi_WaitReady(WRITE_PAGE_TIME_MS);

        buf += bytes;
        addr += bytes;
        len -= bytes;
    }
    SpiFlashi_WriteDisable();
}

void SpiFlash_Erase(void)
{
    SpiFlashi_WriteEnable();
    SpiFlashi_Cmd(CMD_CE);
    SpiFlashi_WaitReady(CHIP_ERASE_TIME_MS);
    SpiFlashi_WriteDisable();
}

void SpiFlash_EraseSector(uint32_t addr)
{
    SpiFlashi_WriteEnable();
    SpiFlashi_CmdWithAddr(CMD_SE, addr, 0, true);
    SpiFlashi_WaitReady(PAGE_ERASE_TIME_MS);
    SpiFlashi_WriteDisable();
}

/** @} */
