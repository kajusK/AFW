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
 * @file    hal/flash.c
 * @brief   Internal flash memory driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/desig.h>
#include "hal/flash.h"

uint32_t Flashd_GetFlashSize(void)
{
    return desig_get_flash_size()*1024;
}

uint32_t Flashd_GetPageSize(void)
{
    /* Might not be valid for all MCUs, but should work for at least for F0 */
    if (desig_get_flash_size() >= 128) {
        return 2048;
    }
    return 1024;
}

void Flashd_WriteEnable(void)
{
    flash_unlock();
}

void Flashd_WriteDisable(void)
{
    flash_lock();
}

void Flashd_ErasePage(uint32_t addr)
{
    flash_erase_page(addr);
}

void Flashd_Write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    while (len >= 2) {
        flash_program_half_word(addr, *((uint16_t *) buf));
        addr += 2;
        buf += 2;
        len -= 2;
    }
    /* len was not multiply of two */
    if (len != 0) {
        flash_program_half_word(addr, *buf << 8 | 0xff);
    }
}

/** @} */
