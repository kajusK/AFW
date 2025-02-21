/**
 * @file    hal/spi.h
 * @brief   SPI driver
 */

#ifndef __HAL_SPI_H
#define __HAL_SPI_H

#include <types.h>

/** Possible prescaler values */
typedef enum {
	SPID_PRESC_2 = 0,
	SPID_PRESC_4 = 1,
	SPID_PRESC_8 = 2,
	SPID_PRESC_16 = 3,
	SPID_PRESC_32 = 4,
	SPID_PRESC_64 = 5,
	SPID_PRESC_128 = 6,
	SPID_PRESC_256 = 7,
} spid_prescaler_t;

typedef enum {
	SPI_MODE_0,
	SPI_MODE_1,
	SPI_MODE_2,
	SPI_MODE_3
} spid_mode_t;

/**
 * Send and receive single byte over SPI
 *
 * @param device	Device ID (1 to 6)
 * @param data		Byte to send
 * @return Byte received
 */
extern uint8_t SPId_Transceive(uint8_t device, uint8_t data);

/**
 * Send data over spi
 *
 * @param device	Device ID (1 to 6)
 * @param [in] buf	Pointer to data buffer with data to send
 * @param len		Amount of bytes to send
 */
extern void SPId_Send(uint8_t device, const uint8_t *buf, size_t len);

/**
 * Read data from spi
 *
 * @param device	Device ID (1 to 6)
 * @param [out] buf	Pointer to data buffer where data will be stored
 * @param len		Amount of bytes to receive
 */
extern void SPId_Receive(uint8_t device, uint8_t *buf, size_t len);

/**
 * Get currently set clock prescaler
 *
 * @param device	Device ID (1 to 6)
 */
spid_prescaler_t SPId_GetPrescaler(uint8_t device);

/**
 * Set clock prescaler
 *
 * @param device	Device ID (1 to 6)
 * @param prescaler Clock prescaler
 */
void SPId_SetPrescaler(uint8_t device, spid_prescaler_t prescaler);

/**
 * Initialize SPI device
 *
 * GPIO pins are not initialized and must be initialize separately
 *
 * @param device	Device ID (1 to 6)
 * @param prescaler Clock prescaler
 * @param mode      SPI mode to use
 * @return True if init was successful
 */
extern void SPId_Init(uint8_t device, spid_prescaler_t prescaler,
		spid_mode_t mode);

#endif
