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
#include <utils/assert.h>
#include "hal/adc.h"

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

uint16_t Adcd_ReadRaw(uint8_t channel)
{
    adc_set_regular_sequence(ADC1, 1, &channel);
    adc_start_conversion_regular(ADC1);
    while (!adc_eoc(ADC1)) {
        ;
    }

    return adc_read_regular(ADC1);
}

uint16_t Adcd_RawToMv(uint16_t raw)
{
    return (uint32_t) adcdi_vdda_mv * raw / ADC_MAX;
}

uint16_t Adcd_RawToVcc(uint16_t raw)
{
    return 3300U * VREFINT_CAL/raw;
}

int8_t Adcd_RawToTemp(uint16_t raw)
{
    uint16_t ref_mv = adcdi_vdda_mv;
    int32_t temp;

    temp = ((uint32_t) TEMP30_CAL - ((uint32_t) raw * ref_mv / 3300))*1000;
    temp = (temp / TEMP_SLOPE) + 30;
    return temp;
}

void Adcd_UpdateVddaRaw(uint16_t raw)
{
    adcdi_vdda_mv = Adcd_RawToVcc(raw);
}

void Adcd_Sleep(void)
{
    adc_disable_temperature_sensor();
    adc_disable_vrefint();
    adc_power_off(ADC1);
}

void Adcd_Wakeup(void)
{
    adc_power_on(ADC1);
    adc_enable_temperature_sensor();
    adc_enable_vrefint();
}

bool Adcd_DmaDataValid(void)
{
    bool ret = dma_get_interrupt_flag(DMA1, ADC1_DMA_CHANNEL, DMA_TCIF);
    dma_clear_interrupt_flags(DMA1, ADC1_DMA_CHANNEL, DMA_TCIF);
    return ret;
}

void Adcd_InitDma(uint16_t *data, const uint8_t *channels, uint8_t length)
{
    ASSERT_NOT(data == NULL || channels == NULL || length < 1 || length > 18);

    Adcd_Init();
    adc_power_off(ADC1);
    adc_set_operation_mode(ADC1, ADC_MODE_SCAN_INFINITE);
    adc_set_regular_sequence(ADC1, length, (uint8_t *) channels);

    rcc_periph_clock_enable(RCC_DMA1);
    dma_channel_reset(DMA1, ADC1_DMA_CHANNEL);
    dma_set_priority(DMA1, ADC1_DMA_CHANNEL, DMA_CCR_PL_LOW);
    dma_set_number_of_data(DMA1, ADC1_DMA_CHANNEL, length);

    dma_set_peripheral_address(DMA1, ADC1_DMA_CHANNEL, (uint32_t)&ADC_DR(ADC1));
    dma_set_peripheral_size(DMA1, ADC1_DMA_CHANNEL, DMA_CCR_PSIZE_16BIT);
    dma_disable_peripheral_increment_mode(DMA1, ADC1_DMA_CHANNEL);
    dma_set_read_from_peripheral(DMA1, ADC1_DMA_CHANNEL);

    dma_set_memory_address(DMA1, ADC1_DMA_CHANNEL, (uint32_t) data);
    dma_set_memory_size(DMA1, ADC1_DMA_CHANNEL, DMA_CCR_MSIZE_16BIT);
    dma_enable_memory_increment_mode(DMA1, ADC1_DMA_CHANNEL);

    dma_enable_channel(DMA1, ADC1_DMA_CHANNEL);
    dma_enable_circular_mode(DMA1, ADC1_DMA_CHANNEL);
    adc_enable_dma_circular_mode(ADC1);
    adc_enable_dma(ADC1);

    adc_power_on(ADC1);
    dma_clear_interrupt_flags(DMA1, ADC1_DMA_CHANNEL, DMA_TCIF);
    adc_start_conversion_regular(ADC1);
}

void Adcd_Init(void)
{
    rcc_periph_clock_enable(RCC_ADC);

    adc_power_off(ADC1);
    adc_calibrate(ADC1);

    adc_set_clk_source(ADC1, ADC_CLKSOURCE_PCLK_DIV4);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPTIME_071DOT5);
    adc_set_resolution(ADC1, ADC_RESOLUTION_12BIT);
    adc_set_operation_mode(ADC1, ADC_MODE_SCAN);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_right_aligned(ADC1);
    adc_disable_analog_watchdog(ADC1);
    adc_enable_temperature_sensor();
    adc_enable_vrefint();
    adc_power_on(ADC1);
}

/** @} */
