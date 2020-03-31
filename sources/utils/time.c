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
 * @file    utils/time.c
 * @brief   Usuall time measurement utilities - millis, etc.
 *
 * @addtogroup utils
 * @{
 */

#include "hal/systick.h"
#include "utils/time.h"

static volatile uint32_t timei_elapsed_ms;

static void Timei_Systick(void)
{
    timei_elapsed_ms++;
}

uint32_t millis(void)
{
    return timei_elapsed_ms;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();

    while ((millis() - start) < ms) {
        ;
    }
}

void Time_Init(void)
{
    Systickd_Init();
    Systickd_SetCallback(Timei_Systick);
}

/** @} */
