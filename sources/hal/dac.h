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
 * @file    hal/dac.h
 * @brief   DAC driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_DAC_H
#define __HAL_DAC_H

#include <types.h>

typedef enum {
    DAC_CHANNEL_1,
    /* only channel 1 available on the low cost devices */
    DAC_CHANNEL_2,
    DAC_CHANNEL_DUAL
} dac_channel_t;

/**
 * Set DAC output channel to required voltage
 *
 * @param channel       Channel to set voltage on
 * @param voltage_mv    Voltage to set on the channel
 */
extern void Dacd_SetMv(dac_channel_t channel, uint16_t voltage_mv);

/**
 * Set Vdda voltage used for calculation of the correct output value
 *
 * @param vdda_mv       Voltage of the DAC power supply (Vdda)
 */
extern void Dacd_UpdateVdda(uint16_t vdda_mv);

/**
 * Enable the DAC channel
 *
 * Once the channel is enabled, the output pin is connected automatically!
 * Set it to analog first
 *
 * @param channel      DAC channel to enable
 */
extern void Dacd_Enable(dac_channel_t channel);

/**
 * Disable the DAC channel
 *
 * @param channel      DAC channel to disable
 */
extern void Dacd_Disable(dac_channel_t channel);

/**
 * Initialize the DAC peripheral
 *
 * @param vdda_mv       Vdda supply voltage level (to calculate output correctly)
 */
extern void Dacd_Init(uint16_t vdda_mv);

#endif

/** @} */
