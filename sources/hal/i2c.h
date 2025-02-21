/**
 * @file    hal/i2c.h
 * @brief   I2C driver
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
