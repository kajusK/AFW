/*
 * Copyright (C) 2024 Jakub Kaderka
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
 * @file    drivers/sd_spi.h
 * @brief   SD card SPI driver for FatFS library
 * @see     elm-chan.org/fsw/ff/doc
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_SD_SPI_H
#define __DRIVERS_SD_SPI_H

#include <types.h>

/*
 * The get_fattime function should be implemented to have a valid file timestamp
 * See http://elm-chan.org/fsw/ff/doc/fattime.html for more information
 *
 * This driver contains a dummy __weak__ implementation that can be easily
 * overriden
 */

/**
 * Notify the FatFS the SD card was removed/inserted
 *
 * @param present   True if SD card was just inserted, false if just removed
 */
extern void SdCard_SetInsertion(bool present);

/**
 * Initialize the sd card driver for FatFs usage
 *
 * @param spi		SPI device to be used
 * @param cs_port	Port of CS pin
 * @param cs_pad	Number of CS pin
 */
extern void SdCard_Init(uint8_t spi, uint32_t cs_port, uint8_t cs_pad);

#endif

/** @} */