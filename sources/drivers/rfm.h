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
 * @param power     Required power from 2 to 20 dBm
 */
extern void RFM_SetPowerDBm(int8_t power);

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
 * @param bandwidth     Bandwidth selection
 * @param sf            Spreading factor
 */
extern void RFM_SetLoraParams(rfm_bw_t bandwidth, rfm_sf_t sf);


/**
 * Configure region we are in (sets frequency range)
 *
 * @param region        Region settings to be used
 */
extern void RFM_SetLoraRegion(rfm_lora_region_t region);

/**
 * Send raw Lora modulated data
 *
 * @param data      Data to be sent
 * @param len       Length of the data buffer
 */
extern void RFM_LoraSend(const uint8_t *data, size_t len);

/**
 * Initialize RFM module in lora mode
 *
 * @return True if module is initialized, false if not responding
 */
extern bool RFM_LoraInit(void);

#endif
/** @} */
