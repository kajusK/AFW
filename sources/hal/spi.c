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
 * @file    hal/spi.c
 * @brief   SPI driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>

#include "utils/assert.h"
#include "hal/spi.h"

static const uint32_t spidi_regs[] = {
		SPI1,
#ifdef SPI2_BASE
		SPI2,
#endif
#ifdef SPI3_BASE
		SPI3,
#endif
};

static const uint32_t spidi_rcc[] = {
		RCC_SPI1,
#ifdef SPI2_BASE
		RCC_SPI2,
#endif
#ifdef SPI3_BASE
		RCC_SPI3,
#endif
};

/**
 * Get SPI device address from device id
 *
 * @param device	Device ID (1 to 6)
 * @return Address of the device's base register
 */
static uint32_t SPIdi_GetDevice(uint8_t device)
{
	ASSERT_NOT(device == 0 || device > sizeof(spidi_regs)/sizeof(spidi_regs[0]));
	return spidi_regs[device - 1];
}

/**
 * Get SPI device rcc register
 *
 * @param device	Device ID (1 to 6)
 * @return Address of the device's rcc register
 */
static enum rcc_periph_clken SPIdi_GetRcc(uint8_t device)
{
	ASSERT_NOT(device == 0 || device > sizeof(spidi_rcc)/sizeof(spidi_rcc[0]));
	return spidi_rcc[device - 1];
}

/**
 * 8 bit version of spi_xfer as on f0, writing to SPI_DR causes spi to send 16
 * bits instead of 8
 *
 * @param spi   Address of the spi peripheral
 * @param data  Data to be sent
 * @return data received
 */
static uint8_t SPId_Xfer8(uint32_t spi, uint8_t data)
{
    SPI_DR8(spi) = data;
    while (SPI_SR(spi) & SPI_SR_BSY) {
        ;
    }
    return SPI_DR8(spi);
}

uint8_t SPId_Transceive(uint8_t device, uint8_t data)
{
	uint32_t spi = SPIdi_GetDevice(device);

	return SPId_Xfer8(spi, data);
}

void SPId_Send(uint8_t device, const uint8_t *buf, size_t len)
{
	uint32_t spi = SPIdi_GetDevice(device);

	while (len-- != 0) {
		SPId_Xfer8(spi, *buf++);
	}
}

void SPId_Receive(uint8_t device, uint8_t *buf, size_t len)
{
	uint32_t spi = SPIdi_GetDevice(device);

	while (len-- != 0) {
		*buf++ = SPId_Xfer8(spi, 0xff);
	}
}

void SPId_Init(uint8_t device, spid_prescaler_t prescaler, spid_mode_t mode)
{
	enum rcc_periph_clken rcc = SPIdi_GetRcc(device);
	uint32_t spi = SPIdi_GetDevice(device);

	rcc_periph_clock_enable(rcc);
	spi_reset(spi);

	spi_set_master_mode(spi);
	spi_set_baudrate_prescaler(spi, prescaler);

	switch (mode) {
		case SPI_MODE_0:
			spi_set_clock_polarity_0(spi);
			spi_set_clock_phase_0(spi);
			break;
		case SPI_MODE_1:
			spi_set_clock_polarity_0(spi);
			spi_set_clock_phase_1(spi);
			break;
		case SPI_MODE_2:
			spi_set_clock_polarity_1(spi);
			spi_set_clock_phase_0(spi);
			break;
		case SPI_MODE_3:
			spi_set_clock_polarity_1(spi);
			spi_set_clock_phase_1(spi);
			break;
	}

	spi_set_full_duplex_mode(spi);
	spi_set_unidirectional_mode(spi);
	spi_set_data_size(spi, SPI_CR2_DS_8BIT);
	spi_send_msb_first(spi);
	spi_fifo_reception_threshold_8bit(spi);

	/* CS controlled manually, but this should be set anyway */
	spi_enable_software_slave_management(spi);
	spi_enable_ss_output(spi);
	spi_set_nss_high(spi);

	spi_enable(spi);
}

/** @} */
