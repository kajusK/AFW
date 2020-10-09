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
 * @file    drivers/rfm.h
 * @brief   Driver for Hope RF RFM95 and similar RF modules
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_RFM_H
#define __DRIVERS_RFM_H

#include <types.h>

/* Signal bandwidth */
typedef enum {
    RFM_BW_125k,
    RFM_BW_250k,
    RFM_BW_500k,
} rfm_bw_t;

/** Spreading factor */
typedef enum {
    RFM_SF_7 = 7,
    RFM_SF_8 = 8,
    RFM_SF_9 = 9,
    RFM_SF_10 = 10,
    RFM_SF_11 = 11,
    RFM_SF_12 = 12,
} rfm_sf_t;

/** Lora regions (frequency to use) */
typedef enum {
    RFM_REGION_AU915,
    RFM_REGION_EU863,
    RFM_REGION_US902,
    RFM_REGION_AS920
} rfm_lora_region_t;

/** The RFM device descriptor */
typedef struct {
    uint8_t spi_device;     /**< SPI device the RFM is connected to */
    uint32_t cs_port;       /**< MCU port the CS signal is connected to */
    uint8_t cs_pad;         /**< MCU pin the CS signal is connected to */
    uint32_t reset_port;    /**< MCU port the reset signal is connected to */
    uint8_t reset_pad;      /**< MCU pin the reset signal is connected to */
    uint32_t io0_port;      /**< MCU port the IO0 signal is connected to */
    uint32_t io0_pad;       /**< MCU pin the IO0 signal is connected to */
    const uint8_t (*region)[3];
} rfm_desc_t;

/**
 * Set transmit power
 *
 * Output range is from -4 to 14dBm, W versions can use up to 20 dBm. Over
 * 17 dBm, continuous operation is not possible. Up to 1% duty cycle and antenna
 * with VSWR up to 3:1 should be used.
 *
 * As the RFM95 module doesn't seem to have RFO output connected, PA must
 * be enabled all the time, therefore the usable range starts really at 2 dBm
 *
 * @param desc      The RFM device descriptor
 * @param power     Required power from 2 to 20 dBm
 */
extern void RFM_SetPowerDBm(const rfm_desc_t *desc, int8_t power);

/**
 * Set Lora transmission bandwidth and spreading factor
 *
 * Use https://www.thethingsnetwork.org/airtime-calculator to calculate
 * required airtime (and also check max amount of bytes and possible datarates
 * for each region).
 *
 * EU supports only 125 and 250 kHz (only SF7 supported here).
 * Higher SF -> lower baudrate -> higher sensitivity -> longer airtime
 * Higher BW -> higher baudrate -> lower sensitivity -> shorter airtime
 *
 * @param desc          The RFM device descriptor
 * @param bandwidth     Bandwidth selection
 * @param sf            Spreading factor
 */
extern void RFM_SetLoraParams(const rfm_desc_t *desc, rfm_bw_t bandwidth,
        rfm_sf_t sf);

/**
 * Configure region we are in (sets frequency range)
 *
 * @param desc      The RFM device descriptor
 * @param region        Region settings to be used
 */
extern void RFM_SetLoraRegion(rfm_desc_t *desc, rfm_lora_region_t region);

/**
 * Send raw Lora modulated data
 *
 * @param desc      The RFM device descriptor
 * @param data      Data to be sent
 * @param len       Length of the data buffer
 */
extern void RFM_LoraSend(const rfm_desc_t *desc, const uint8_t *data,
        size_t len);

/**
 * Initialize RFM module in lora mode
 *
 * @param [out] desc    The RFM device descriptor
 * @param spi_device    The SPI device to use
 * @param cs_port       Port of the CS pin
 * @param cs_pad        Pin of the CS pin
 * @param reset_port    Port of the RESET pin
 * @param reset_pad     Pin of the RESET pin
 * @param io0_port      Port of the IO0 pin
 * @param io0_pad       Pin of the IO0 pin
 * @return True if module is initialized, false if not responding
 */
extern bool RFM_LoraInit(rfm_desc_t *desc, uint8_t spi_device, uint32_t cs_port,
        uint8_t cs_pad, uint32_t reset_port, uint8_t reset_pad,
        uint32_t io0_port, uint8_t io0_pad);

#endif
/** @} */
