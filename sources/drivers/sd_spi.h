/**
 * @file    drivers/sd_spi.h
 * @brief   SD card SPI driver
 */

#ifndef __DRIVERS_SD_SPI_H
#define __DRIVERS_SD_SPI_H

#include <types.h>

/** Card sector size in bytes */
#define SDSPI_SECTOR_SIZE_B 512

/** SD Card device descriptor */
typedef struct {
    uint8_t device;    /**< SPI device */
    uint32_t cs_port;  /**< Port of CS pin */
    uint32_t cs_pad;   /**< Pad of CS pin */
    bool present;      /**< True if card is inserted */
    uint8_t card_type; /**< Type of the inserted SD card, 0 for no card present */
} sdspi_desc_t;

/**
 * Initialize new device and put it into ready state
 *
 * @param desc	Card descriptor
 * @return True if initialized, false if not responding, not found...
 */
bool SDSPI_InitCard(sdspi_desc_t *desc);

/**
 * Read data from storage device
 *
 * @param desc	    Card descriptor
 * @param buff		Read data buffer
 * @param sector	Start sector number
 * @param count		Number of sectors to read
 * @return True on success, false if not responding, timed out,...
 */
bool SDSPI_ReadSector(sdspi_desc_t *desc, uint8_t *buff, uint32_t sector, uint32_t count);

/**
 * Write data to storage device
 *
 * @param desc	    Card descriptor
 * @param buff		Write data buffer
 * @param sector	Start sector number
 * @param count		Number of sectors to write
 * @return True on success, false if not responding, timed out,...
 */
bool SDSPI_WriteSector(sdspi_desc_t *desc, const uint8_t *buff, uint32_t sector, uint32_t count);

/**
 * Make sure there's no pending write process
 *
 * @param desc	    Card descriptor
 * @return false if card not responding/still busy
 */
bool SDSPI_Sync(sdspi_desc_t *desc);

/**
 * Get amount of sectors on the card
 *
 * @param desc	    Card descriptor
 * @return 0 if failed, else amount of sectors present
 */
uint32_t SDSPI_GetSectorsCount(sdspi_desc_t *desc);

/**
 * Mark the card as present or removed
 *
 * @param desc	    Card descriptor
 * @param present   True if SD card was just inserted, false if just removed
 */
void SDSPI_SetInserted(sdspi_desc_t *desc, bool present);

/**
 * Check if the SDCard is present in the slot
 */
bool SDSPI_IsInserted(sdspi_desc_t *desc);

/**
 * Check if card was successfully initialized
 */
bool SDSPI_IsInitialized(sdspi_desc_t *desc);

/**
 * Initialize the sd card driver
 *
 * @param desc	    Card descriptor
 * @param spi		SPI device to be used
 * @param cs_port	Port of CS pin
 * @param cs_pad	Number of CS pin
 */
void SDSPI_Init(sdspi_desc_t *desc, uint8_t spi, uint32_t cs_port, uint8_t cs_pad);

#endif
