/**
 * @file    hal/dac.c
 * @brief   DAC driver
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dac.h>
#include "dac.h"

static uint16_t daci_vdda_mv = 3300;

void Dacd_SetMv(dac_channel_t channel, uint16_t voltage_mv)
{
    uint16_t value = ((uint32_t)voltage_mv * 4096) / daci_vdda_mv;
    if (value > 0x0fff) {
        value = 0x0fff;
    }

    dac_load_data_buffer_single(value, RIGHT12, (data_channel)channel);
    dac_software_trigger((data_channel)channel);
}

void Dacd_UpdateVdda(uint16_t vdda_mv)
{
    daci_vdda_mv = vdda_mv;
}

void Dacd_Enable(dac_channel_t channel)
{
    dac_enable((data_channel)channel);
}

void Dacd_Disable(dac_channel_t channel)
{
    dac_disable((data_channel)channel);
}

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
