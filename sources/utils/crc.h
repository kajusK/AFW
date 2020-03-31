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
 * @file    crc.c
 * @brief   CRC calculations
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_CRC_H
#define __UTILS_CRC_H

#include <types.h>

/** Initial value for CRC16 calculation */
#define CRC16_INITIAL_VALUE  0xFFFFU

/**
 * Calculate CRC from initial value and single byte
 *
 * @param [in] buf  Byte to calculate crc for
 * @param [in] crc  Initial CRC value
 *
 * @return CRC 16 (polynomial 0x1021 16)
 */
extern uint16_t CRC16_Add(uint8_t buf, uint16_t crc);

/**
 * Calculate CRC for buffer
 *
 * @param [in] buf  Data to calculate crc for
 * @param [in] len  Length of data buffer
 *
 * @return CRC 8 (polynomial 0x07)
 */
extern uint16_t CRC16(const uint8_t *buf, uint32_t len);

#endif

/** @} */
