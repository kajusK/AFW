/**
 * @file    drivers/diskio_common.h
 * @brief   SD card SPI driver for FatFS/PetitFs
 */

#ifndef __MODULES_DISKIO_H
#define __MODULES_DISKIO_H

#include "drivers/sd_spi.h"

/**
 * Get current timestamp
 *
 * Override in other place in code to provide correct data, only a dummy
 * implementation is here.
 *
 * A valid data must be returned, 1.1.2024 is used here as example.
 *
 * @see http://elm-chan.org/fsw/ff/doc/fattime.html
 *
 * DWORD __attribute__((weak)) get_fattime(void)
 * {
 *     DWORD timestamp = 0;
 *     timestamp |= 1 << 16;             // day 1-12
 *     timestamp |= 1 << 21;             // month 1-12
 *     timestamp |= (2024 - 1980) << 25; // years since 1980
 *
 *     return timestamp;
 * }
 */

/**
 * Point diskio implementation to initialized sd card driver descriptor
 *
 * @note The descriptor memory must remain accessible after call
 */
void Diskio_SetCard(sdspi_desc_t *card_desc);

#endif
