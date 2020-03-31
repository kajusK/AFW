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
 * @file    hal/systick.h
 * @brief   Systick driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_SYSTICK_H
#define __HAL_SYSTICK_H

#include <types.h>

typedef void (*systickd_cb_t)(void);

/**
 * Set callback which should be called upon systick interrupt
 *
 * Called in interrupt!
 *
 * @param cb        Callback to be called
 */
extern void Systickd_SetCallback(systickd_cb_t cb);

/**
 * Initialize systick timer
 *
 * @param frequency Frequency of systick interrupt
 */
extern void Systickd_Init(void);

#endif

/** @} */
