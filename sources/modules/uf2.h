/*
 * Copyright (C) 2020 Jakub Kaderka
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
 * @file    modules/uf2.h
 * @brief   USB FW update using UF2 format
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_UF2_H
#define __MODULES_UF2_H

#include <types.h>

/**
 * Process write request of UF2 data block
 *
 * @param block     UF2 block (512 bytes) to be written
 * @return Successfulness of the operation
 */
extern bool UF2_Write(const uint8_t *data);

/**
 * Read currently running firmware in UF2 data format
 *
 * @param [out] data    Address to store the data to (512 bytes)
 * @param offset        Offset to read in blocks from the first block
 * @return Successfulness of the operation
 */
extern bool UF2_Read(uint8_t *data, uint32_t offset);

/**
 * Get firmware image size
 *
 * @return Size of the UF2 FW image in bytes
 */
extern uint32_t UF2_GetImgSize(void);

#endif

/** @} */
