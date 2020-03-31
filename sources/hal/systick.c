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
 * @file    hal/systick.c
 * @brief   Systick driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

#include "utils/assert.h"
#include "hal/systick.h"

static systickd_cb_t systickdi_cb;

void sys_tick_handler(void)
{
    if (systickdi_cb != NULL) {
        systickdi_cb();
    }
}

void Systickd_SetCallback(systickd_cb_t cb)
{
    systickdi_cb = cb;
}

void Systickd_Init(void)
{
    systick_clear();

    ASSERT(systick_set_frequency(1000, rcc_ahb_frequency));
    systick_counter_enable();
    systick_interrupt_enable();
}

/** @} */
