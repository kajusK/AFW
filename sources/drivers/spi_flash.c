/**
 * @file    drivers/spi_flash.h
 * @brief   Driver for SST26 SPI flash memories
 */

#include <types.h>
#include "utils/time.h"
#include "hal/io.h"
#include "hal/spi.h"

#include "drivers/spi_flash.h"

#define PAGE_BYTES 256
#define CHIP_ERASE_TIME_MS 40
#define PAGE_ERASE_TIME_MS 20
#define WRITE_PAGE_TIME_MS 2

#define cs_set() IOd_SetLine(desc->cs_port, desc->cs_pad, 0)
#define cs_unset() IOd_SetLine(desc->cs_port, desc->cs_pad, 1)

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
 * Send command followed by address and optionally also dummy bytes
 *
 * @param desc      The device descriptor
 * @param cmd       Command
 * @param addr      24 bytes of address
 * @param dummy     Amount (up to 4) of dummy bytes to send after address
 * @param release_cs    Release cs pin after sending data
 */
static void SpiFlashi_CmdWithAddr(const spiflash_desc_t *desc,
        spiflash_cmd_t cmd, uint32_t addr, uint8_t dummy, bool release_cs)
{
    uint8_t data[8] = {0};

    ASSERT_NOT(dummy > 4);

    data[0] = cmd;
    data[1] = addr >> 16;
    data[2] = addr >> 8;
    data[3] = addr;

    cs_set();
    SPId_Send(desc->spi_device, data, 4+dummy);

    if (release_cs) {
        cs_unset();
    }
}

/**
 * Send Flash command without any additional parameters
 *
 * @param desc      The device descriptor
 * @param cmd   Command to be executed
 */
static void SpiFlashi_Cmd(const spiflash_desc_t *desc, spiflash_cmd_t cmd)
{
    cs_set();
    SPId_Send(desc->spi_device, &cmd, 1);
    cs_unset();
}

/**
 * Wait for flash operation to finish
 *
 * @param desc      The device descriptor
 * @param timeout_ms    Time to wait for op to finish
 */
static void SpiFlashi_WaitReady(const spiflash_desc_t *desc,
        uint32_t timeout_ms)
{
    const uint8_t cmd = CMD_RDSR;
    uint32_t start = millis();

    cs_set();
    SPId_Send(desc->spi_device, &cmd, 1);

    while ((millis() - start) < timeout_ms) {
        /* continuously read status register, command send only once */
        if ((SPId_Transceive(desc->spi_device, 0xff) & STATUS_BUSY) == 0) {
            break;
        }
    }

    cs_unset();
}

/**
 * Enable write protection
 *
 * @param desc      The device descriptor
 */
static void SpiFlashi_WriteEnable(const spiflash_desc_t *desc)
{
    SpiFlashi_Cmd(desc, CMD_WREN);
}

/**
 * Disable write protection
 *
 * @param desc      The device descriptor
 */
static void SpiFlashi_WriteDisable(const spiflash_desc_t *desc)
{
    SpiFlashi_Cmd(desc, CMD_WRDI);
}

void SpiFlash_WriteUnlock(const spiflash_desc_t *desc)
{
    uint8_t buf[11] = {0};
    buf[0] = CMD_WBPR;

    SpiFlashi_WriteEnable(desc);
    cs_set();
    SPId_Send(desc->spi_device, buf, sizeof(buf));
    cs_unset();
    SpiFlashi_WriteDisable(desc);
}

void SpiFlash_Read(const spiflash_desc_t *desc, uint32_t addr, uint8_t *buf,
        size_t len)
{
    SpiFlashi_CmdWithAddr(desc, CMD_READ, addr, 0, false);
    SPId_Receive(desc->spi_device, buf, len);
    cs_unset();
}

void SpiFlash_Write(const spiflash_desc_t *desc, uint32_t addr,
        const uint8_t *buf, size_t len)
{
    uint16_t bytes;

    /* up to 256 bytes in single write */
    while (len != 0) {
        if (len >= PAGE_BYTES) {
            bytes = PAGE_BYTES;
        } else {
            bytes = len;
        }
        SpiFlashi_WriteEnable(desc);
        SpiFlashi_CmdWithAddr(desc, CMD_PP, addr, 0, false);
        SPId_Send(desc->spi_device, buf, bytes);
        cs_unset();
        SpiFlashi_WaitReady(desc, WRITE_PAGE_TIME_MS);

        buf += bytes;
        addr += bytes;
        len -= bytes;
    }
    SpiFlashi_WriteDisable(desc);
}

void SpiFlash_Erase(const spiflash_desc_t *desc)
{
    SpiFlashi_WriteEnable(desc);
    SpiFlashi_Cmd(desc, CMD_CE);
    SpiFlashi_WaitReady(desc, CHIP_ERASE_TIME_MS);
    SpiFlashi_WriteDisable(desc);
}

void SpiFlash_EraseSector(const spiflash_desc_t *desc, uint32_t addr)
{
    SpiFlashi_WriteEnable(desc);
    SpiFlashi_CmdWithAddr(desc, CMD_SE, addr, 0, true);
    SpiFlashi_WaitReady(desc, PAGE_ERASE_TIME_MS);
    SpiFlashi_WriteDisable(desc);
}

void SpiFlash_Init(spiflash_desc_t *desc, uint8_t spi_device, uint32_t cs_port,
        uint8_t cs_pad)
{
    desc->spi_device = spi_device;
    desc->cs_port = cs_port;
    desc->cs_pad = cs_pad;
}

/** @} */
