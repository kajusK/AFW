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

/**
 * Unlock write-protection registers for all memory addresses
 *
 * Must be called once after boot before writing to flash memory
 */
extern void SpiFlash_WriteUnlock(void);

/**
 * Read memory content
 *
 * @param addr      Address to read from
 * @param [out] buf Buffer to read data to
 * @param len       Amount of bytes to read
 */
extern void SpiFlash_Read(uint32_t addr, uint8_t *buf, size_t len);

/**
 * Write data to memory
 *
 * @param addr      Address to write to
 * @param [out] buf Data to be written
 * @param len       Amount of bytes to write
 */
extern void SpiFlash_Write(uint32_t addr, const uint8_t *buf, size_t len);

/**
 * Erase whole memory
 */
extern void SpiFlash_Erase(void);

/**
 * Erase memory sector
 */
extern void SpiFlash_EraseSector(uint32_t addr);

#endif

/** @} */
