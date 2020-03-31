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
 * @file    hal/adc.c
 * @brief   ADC driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/dma.h>

#include "hal/adc.h"

#define ADC_TEMP_CHANNEL    16
#define ADC_INT_REF_CHANNEL 17

#define ADC1_DMA_CHANNEL DMA_CHANNEL1

/* Resolution of adc is 12 bits by default - 2^12 */
#define ADC_MAX 4096

/** average temperature sensor slope in 3,3 V/°C multiplied by 1000 */
#define TEMP_SLOPE 5336
/** Calibration value for internal temperature sensor at 30 deg C */
#define TEMP30_CAL (*((uint16_t*) ((uint32_t) 0x1FFFF7B8)))
/** Calibration value for internal reference */
#define VREFINT_CAL (*((uint16_t *) ((uint32_t) 0x1FFFF7BA)))

/** Current vdda voltage (can be measured by adc) */
static uint16_t adcdi_vdda_mv = 3300;

/**
 * Read raw output from adc measurement
 *
 * @param channel   Channel to read
 * @return Raw adc 12 bit value
 */
static uint16_t Adcdi_ReadRaw(uint8_t channel)
{
    adc_set_regular_sequence(ADC1, 1, &channel);
    adc_start_conversion_regular(ADC1);
    while (!adc_eoc(ADC1)) {
        ;
    }

    return adc_read_regular(ADC1);
}

uint16_t Adcd_ReadMv(uint8_t channel)
{
    uint16_t raw = Adcdi_ReadRaw(channel);

    return (uint32_t) adcdi_vdda_mv * raw / ADC_MAX;
}

uint16_t Adcd_ReadVccMv(void)
{
    uint16_t raw = Adcdi_ReadRaw(ADC_INT_REF_CHANNEL);
    return 3300U * VREFINT_CAL/raw;
}

int8_t Adcd_ReadTempDegC(void)
{
    uint16_t raw = Adcdi_ReadRaw(ADC_TEMP_CHANNEL);
    uint16_t ref_mv = adcdi_vdda_mv;
    int32_t temp;

    temp = ((uint32_t) TEMP30_CAL - ((uint32_t) raw * ref_mv / 3300))*1000;
    temp = (temp / TEMP_SLOPE) + 30;
    return temp;
}

void Adcd_UpdateVdda(void)
{
    adcdi_vdda_mv = Adcd_ReadVccMv();
}

void Adcd_Sleep(void)
{
    adc_power_off(ADC1);
    adc_disable_temperature_sensor();
    adc_disable_vrefint();
}

void Adcd_Wakeup(void)
{
    adc_enable_temperature_sensor();
    adc_enable_vrefint();
    adc_power_on(ADC1);

    /* Wait for ADC starting up. */
    for (int i = 0; i < 800000; i++) {
        __asm__("nop");
    }
    Adcd_UpdateVdda();
}

void Adcd_Init(void)
{
    rcc_periph_clock_enable(RCC_ADC);

    adc_power_off(ADC1);
    adc_enable_temperature_sensor();
    adc_enable_vrefint();
    adc_set_clk_source(ADC1, ADC_CLKSOURCE_ADC);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPTIME_071DOT5);
    adc_set_resolution(ADC1, ADC_RESOLUTION_12BIT);
    adc_set_operation_mode(ADC1, ADC_MODE_SCAN);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_right_aligned(ADC1);
    adc_disable_analog_watchdog(ADC1);
    adc_calibrate(ADC1);
    adc_power_on(ADC1);

    /* Wait for ADC starting up. */
    for (int i = 0; i < 800000; i++) {
        __asm__("nop");
    }

    Adcd_UpdateVdda();
}

/** @} */
