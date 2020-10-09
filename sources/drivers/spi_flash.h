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

#ifndef __DRIVERS_SPI_FLASH_H
#define __DRIVERS_SPI_FLASH_H

#include <types.h>

typedef struct {
    uint8_t spi_device;         /**< SPI device the flash is connected to */
    uint32_t cs_port;           /**< MCU port the CS is connected to */
    uint8_t cs_pad;             /**< MCU pin the CS is connected to */
} spiflash_desc_t;

/**
 * Unlock write-protection registers for all memory addresses
 *
 * Must be called once after boot before writing to flash memory
 *
 * @param desc      The device descriptor
 */
extern void SpiFlash_WriteUnlock(const spiflash_desc_t *desc);

/**
 * Read memory content
 *
 * @param desc      The device descriptor
 * @param addr      Address to read from
 * @param [out] buf Buffer to read data to
 * @param len       Amount of bytes to read
 */
extern void SpiFlash_Read(const spiflash_desc_t *desc, uint32_t addr,
        uint8_t *buf, size_t len);

/**
 * Write data to memory
 *
 * @param desc      The device descriptor
 * @param addr      Address to write to
 * @param [out] buf Data to be written
 * @param len       Amount of bytes to write
 */
extern void SpiFlash_Write(const spiflash_desc_t *desc, uint32_t addr,
        const uint8_t *buf, size_t len);

/**
 * Erase whole memory
 *
 * @param desc      The device descriptor
 */
extern void SpiFlash_Erase(const spiflash_desc_t *desc);

/**
 * Erase memory sector
 *
 * @param desc      The device descriptor
 */
extern void SpiFlash_EraseSector(const spiflash_desc_t *desc, uint32_t addr);

/**
 * Initialize the memory
 *
 * @param [out] desc        The device descriptor
 * @param spi_device        The SPI device to use
 * @param cs_port           MCU port with CS pin
 * @param cs_pad            MCU pin with CS signal
 */
extern void SpiFlash_Init(spiflash_desc_t *desc, uint8_t spi_device,
        uint32_t cs_port, uint8_t cs_pad);

#endif

/** @} */
