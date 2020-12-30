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
 * @file    hal/comp.c
 * @brief   Analog comparator driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include "comp.h"

#define REG_COMP_CSR      MMIO32(SYSCFG_COMP_BASE + 0x1c)

void Compd_Enable(uint8_t channel)
{
    REG_COMP_CSR |= COMP_CSR_EN << channel*16;
}

void Compd_Disable(uint8_t channel)
{
    REG_COMP_CSR &= ~(COMP_CSR_EN << channel*16);
}

void Compd_Init(uint8_t channel, comp_speed_t speed, comp_hyst_t hyst,
        comp_in_neg_t input, comp_out_t output, bool invert)
{
    rcc_periph_clock_enable(RCC_SYSCFG_COMP);
    REG_COMP_CSR &= ~(0xffff << channel*16);
    REG_COMP_CSR |= speed << channel*16;
    REG_COMP_CSR |= hyst << channel*16;
    REG_COMP_CSR |= input << channel*16;
    REG_COMP_CSR |= output << channel*16;

    if (invert) {
        REG_COMP_CSR |= COMP_CSR_POL << channel*16;
    }
}

/** @} */
