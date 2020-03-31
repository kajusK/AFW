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
 * @file    modules/fw.h
 * @brief   Firmware upgrade module
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_FW_H
#define __MODULES_FW_H

#include <types.h>

/**
 * Find latest valid image and boot it
 */
extern void Fw_Run(void);

/**
 * Reboot the MCU
 */
extern void Fw_Reboot(void);

/**
 * Initialize fw update and erase fw area
 *
 * @param major     Major fw version
 * @param minor     Minor fw version
 * @param crc       Expected CRC of the image
 * @param len       Length of the image
 *
 * @return True if succeeded
 */
extern bool Fw_UpdateInit(uint8_t major, uint8_t minor, uint16_t crc, uint32_t len);

/**
 * Write update data to selected address
 *
 * @param addr      Address in the image area to write data to
 * @param buf       Data to be written
 * @param len       Length of the data
 */
extern void Fw_Update(uint32_t addr, const uint8_t *buf, uint32_t len);

/**
 * Finish the fw update - check final crc, write headers,...
 *
 * @return true if suceeded (crc matches,...)
 */
extern bool Fw_UpdateFinish(void);

#endif

/** @} */
