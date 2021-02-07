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
 * @file    hal/flash.h
 * @brief   Internal flash memory driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_FLASH_H_
#define __HAL_FLASH_H_

#include <types.h>
#include <libopencm3/stm32/memorymap.h>

/** Flash address memory map */
#define FLASHD_START              FLASH_BASE

/**
 * Get size of the flash memory in the MCU in bytes
 */
extern uint32_t Flashd_GetFlashSize(void);

/**
 * Get size of the memory page in the MCU in bytes
 */
extern uint32_t Flashd_GetPageSize(void);

/**
 * Enable writing to the flash memory
 */
extern void Flashd_WriteEnable(void);

/**
 * Disable writing to the flash memory
 */
extern void Flashd_WriteDisable(void);

/**
 * Erase given flash page
 */
extern void Flashd_ErasePage(uint32_t addr);

/**
 * Write data to the internal flash
 *
 * Unfortunately the stm32 limits writes to even addresses in 2 byte chunks,
 * once the data on the address is not 0xffff, it cannot be updated again.
 *
 * @param addr  Start address (must be an even address)
 * @param buf   Data buffer
 * @param len   Amount of bytes to be written
 */
extern void Flashd_Write(uint32_t addr, const uint8_t *buf, uint32_t len);

#endif

/** @} */
