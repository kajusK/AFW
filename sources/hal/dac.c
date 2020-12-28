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
 * @file    hal/dac.c
 * @brief   DAC driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dac.h>
#include "dac.h"

static uint16_t daci_vdda_mv = 3300;

/**
 * Set DAC output channel to required voltage
 *
 * @param channel       Channel to set voltage on
 * @param voltage_mv    Voltage to set on the channel
 */
void Dacd_SetMv(dac_channel_t channel, uint16_t voltage_mv)
{
    uint16_t value = ((uint32_t)voltage_mv * 4096)/daci_vdda_mv;
    if (value > 0x0fff) {
        value = 0x0fff;
    }

    dac_load_data_buffer_single(value, RIGHT12, (data_channel)channel);
    dac_software_trigger((data_channel)channel);
}

/**
 * Set Vdda voltage used for calculation of the correct output value
 *
 * @param vdda_mv       Voltage of the DAC power supply (Vdda)
 */
void Dacd_UpdateVdda(uint16_t vdda_mv)
{
    daci_vdda_mv = vdda_mv;
}


/**
 * Enable the DAC channel
 *
 * Once the channel is enabled, the output pin is connected automatically!
 * Set it to analog first
 *
 * @param channel      DAC channel to enable
 */
void Dacd_Enable(dac_channel_t channel)
{
    dac_enable((data_channel)channel);
}

/**
 * Disable the DAC channel
 *
 * @param channel      DAC channel to disable
 */
void Dacd_Disable(dac_channel_t channel)
{
    dac_disable((data_channel)channel);
}

/**
 * Initialize the DAC peripheral
 *
 * @param vdda_mv       Vdda supply voltage level (to calculate output correctly)
 */
void Dacd_Init(uint16_t vdda_mv)
{
    rcc_periph_clock_enable(RCC_DAC);

    dac_disable_waveform_generation(CHANNEL_1);
    dac_disable_waveform_generation(CHANNEL_2);
    dac_set_trigger_source(DAC_CR_TSEL1_SW);
    dac_set_trigger_source(DAC_CR_TSEL2_SW);
    dac_load_data_buffer_single(0, RIGHT12, CHANNEL_1);
    dac_load_data_buffer_single(0, RIGHT12, CHANNEL_2);

    daci_vdda_mv = vdda_mv;
}

/** @} */
