/**
 * @file diskio_fatfs.c
 * @brief FATFS diskio layer implementation for SPI SD Cards
 */

#include "drivers/sd_spi.h"
#include "ff.h"
#include "diskio.h"
#include "diskio_common.h"

static sdspi_desc_t *sdspi_desc = NULL;

/**
 * Get card descriptor from driver ID
 *
 * @param drv	FatFS Drive ID, only 0 is available
 * @return  Descriptor or NULL if drive not available
 */
static sdspi_desc_t *getDrive(BYTE drv)
{
    if (drv == 0) {
        return sdspi_desc;
    }
    return NULL;
}

/**
 * FatFs get disk status callback
 *
 * @param drv	Drive ID
 */
DSTATUS disk_status(BYTE drv)
{
    sdspi_desc_t *desc = getDrive(drv);
    if (desc == NULL || !SDSPI_IsInserted(desc)) {
        return STA_NODISK;
    }
    if (!SDSPI_IsInitialized(desc)) {
        return STA_NOINIT;
    }
    return 0;
}

/**
 * Initialize device and put it into ready state
 *
 * @param drv	Drive ID
 */
DSTATUS disk_initialize(BYTE drv)
{
    sdspi_desc_t *desc = getDrive(drv);
    if (desc == NULL) {
        return STA_NODISK;
    }
    if (!SDSPI_InitCard(desc)) {
        return STA_NOINIT;
    }
    return 0;
}

/**
 * Read data from storage device
 *
 * @param drv		Drive ID
 * @param buff		Read data buffer
 * @param sector	Start sector number
 * @param count		Number of sectors to read
 */
DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
    sdspi_desc_t *desc = getDrive(drv);
    if (desc == NULL || disk_status(drv) != 0) {
        return RES_NOTRDY;
    }

    if (!SDSPI_ReadSector(desc, buff, sector, count)) {
        return RES_ERROR;
    }
    return RES_OK;
}

/**
 * Write data to storage device
 *
 * @param drv		Drive ID
 * @param buff		Write data buffer
 * @param sector	Start sector number
 * @param count		Number of sectors to write
 */
DRESULT disk_write(BYTE drv, const BYTE *buff, DWORD sector, UINT count)
{
    sdspi_desc_t *desc = getDrive(drv);
    if (desc == NULL || disk_status(drv) != 0) {
        return RES_NOTRDY;
    }

    if (!SDSPI_WriteSector(desc, buff, sector, count)) {
        return RES_ERROR;
    }
    return RES_OK;
}

/**
 * Control device specific features
 *
 * @param drv		Drive ID
 * @param ctrl		Control command code
 * @param buff		Parameter and data buffer
 */
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
    DRESULT res = RES_ERROR;
    sdspi_desc_t *desc = getDrive(drv);
    if (desc == NULL || disk_status(drv) != 0) {
        return RES_NOTRDY;
    }

    switch (ctrl) {
        // Make sure that no pending write process
        case CTRL_SYNC:
            if (SDSPI_Sync(desc)) {
                res = RES_OK;
            }
            break;

        // Get number of sectors on the disk (DWORD)
        case GET_SECTOR_COUNT: {
            uint32_t sectors = SDSPI_GetSectorsCount(desc);
            if (sectors != 0) {
                *(DWORD *)buff = sectors;
                res = RES_OK;
            }
            break;
        }

        // Get erase block size in unit of sector (DWORD)
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = 128;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
            break;
    }

    return res;
}

/**
 * Get current timestamp
 *
 * Override in other place in code to provide correct data, only a dummy
 * implementation is here.
 *
 * A valid data must be returned, 1.1.2024 is used here.
 *
 * @see http://elm-chan.org/fsw/ff/doc/fattime.html
 */
DWORD __attribute__((weak)) get_fattime(void)
{
    DWORD timestamp = 0;
    timestamp |= 1 << 16;             // day 1-12
    timestamp |= 1 << 21;             // month 1-12
    timestamp |= (2024 - 1980) << 25; // years since 1980

    return timestamp;
}

void Diskio_SetCard(sdspi_desc_t *card_desc)
{
    sdspi_desc = card_desc;
}
