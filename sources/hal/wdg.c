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
 * @file    hal/wdg.c
 * @brief   Watchdog driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/iwdg.h>

#include "hal/wdg.h"

void Wdgd_Init(uint32_t period_ms)
{
    rcc_periph_clock_enable(RCC_WWDG);
    iwdg_set_period_ms(period_ms);
    iwdg_start();
}

void Wdgd_Clear(void)
{
    iwdg_reset();
}

/** @} */
