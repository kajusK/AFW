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
 * @file    hal/i2c.h
 * @brief   I2C driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_I2C_H
#define __HAL_I2C_H

#include <types.h>

/**
 * Send/receive data over i2c
 *
 * Tx data are sent first and if rxbuf is not NULL, repeated start is send
 * and requested amount of bytes is received
 *
 * @param address      Device address (7 bit)
 * @param [in] txbuf   Data to send or NULL
 * @param txlen        Length of txbuf
 * @param [out] rxbuf  Buffer for received data or NULL
 * @param rxlen        Amount of bytes to receive
 *
 * @TODO add support for more than 255 bytes in both directions
 * @return True if data were acked, False for NACK
 */
extern bool I2Cd_Transceive(uint8_t device, uint8_t address,
		const uint8_t *txbuf, uint8_t txlen, uint8_t *rxbuf, uint8_t rxlen);

/**
 * Initialize the i2c peripheral
 *
 * @param device	Device ID (starting from 1)
 * @param fast		Set to true for 400 kHz, else 100 kHz
 */
extern void I2Cd_Init(uint8_t device, bool fast);

#endif

/** @} */
