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
 * @file    hal/usb_msc.h
 * @brief   USB Mass storage class setup
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_USB_MSC_H
#define __HAL_USB_MSC_H

#include <types.h>
#include "config.h"

/* Default values that can be overridden
#define USBD_VENDOR 0x0483
#define USBD_PRODUCT 0x5720
#define USBD_MANUFACTURE_STR "Deadbadger"
#define USBD_DEVICE_STR "Device"
#define USBD_VERSION_STR "0000"
*/

/**
 * @param lba - Logical block address - 1 = 512, 2 = 1024,...
 * @param buf - Destination buffer to copy 512 bytes to
 */
typedef int (*usb_read_block_t)(uint32_t lba, uint8_t *buf);
typedef int (*usb_write_block_t)(uint32_t lba, const uint8_t *buf);

/**
 * USB processing, call periodically
 */
extern void Usbd_MscPoll(void);

/**
 * Initialize USB and MSC stacks
 *
 * @param blocks    Size of storage area accessible over msc in 512B blocks
 * @param read      Function to read block from disk
 * @param write     Function to write block to disk
 */
extern void Usbd_MscInit(uint32_t blocks, usb_read_block_t read,
        usb_write_block_t write);

#endif

/** @} */
