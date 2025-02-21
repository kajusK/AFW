/**
 * @file    drivers/sd_spi.h
 * @brief   SD card SPI driver for FatFS library
 * @see     elm-chan.org/fsw/ff/doc
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
