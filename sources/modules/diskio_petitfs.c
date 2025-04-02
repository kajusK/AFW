/**
 * @file diskio_petitfs.c
 * @brief PetitFS diskio layer implementation for SPI SD Cards
 */

#include <types.h>
#include <string.h>
#include "pff.h"
#include "diskio.h"
#include "diskio_common.h"
#include "drivers/sd_spi.h"

static sdspi_desc_t *sdspi_desc = NULL;

/**
 * Initialize device and put it into ready state
 */
DSTATUS disk_initialize(void)
{
    if (sdspi_desc == NULL) {
        return STA_NODISK;
    }
    if (!SDSPI_InitCard(sdspi_desc)) {
        return STA_NOINIT;
    }
    return 0;
}

/**
 * Read partial sector
 *
 * @param buff		Buffer to read data to
 * @param sector	Sector number (LBA)
 * @param offset	Offset in the sector in bytes
 * @param count		Amount of bytes to read (bit15:destination)
 */
DRESULT disk_readp(BYTE *buff, DWORD sector, UINT offset, UINT count)
{
    uint8_t data[SDSPI_SECTOR_SIZE_B];
    if (sdspi_desc == NULL || !SDSPI_IsInitialized(sdspi_desc)) {
        return RES_NOTRDY;
    }
    if (offset + count > SDSPI_SECTOR_SIZE_B) {
        return RES_PARERR;
    }
    if (!SDSPI_ReadSector(sdspi_desc, data, sector, 1)) {
        return RES_ERROR;
    }

    uint8_t *pos = &data[offset];
    while (count--) {
        *buff++ = *pos++;
    }
    return RES_OK;
}

/**
 * Write partial sector
 *
 * @param buff	Pointer to the data to be written, NULL:Initiate/Finalize write operation
 * @param sc	Sector number (LBA) or number of bytes to write
 */
DRESULT disk_writep(const BYTE *buff, DWORD sc)
{
    static uint8_t data[SDSPI_SECTOR_SIZE_B];
    static uint16_t offset = 0;
    static uint32_t sector = 0;

    if (sdspi_desc == NULL || !SDSPI_IsInitialized(sdspi_desc)) {
        return RES_NOTRDY;
    }

    if (!buff) {
        if (sc) {
            offset = 0;
            sector = sc;
        } else {
            if (!SDSPI_WriteSector(sdspi_desc, data, sector, 1)) {
                return RES_ERROR;
            }
        }
    } else {
        if (offset + sc > SDSPI_SECTOR_SIZE_B) {
            return RES_PARERR;
        }
        uint8_t *pos = &data[offset];
        while (sc--) {
            *pos++ = *buff++;
        }
    }

    return RES_OK;
}

void Diskio_SetCard(sdspi_desc_t *card_desc)
{
    sdspi_desc = card_desc;
}
