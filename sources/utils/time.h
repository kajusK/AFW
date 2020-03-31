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
 * @file    utils/time.h
 * @brief   Usuall time measurement utilities - millis, etc.
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_TIME_H
#define __UTILS_TIME_H

#include <types.h>

/**
 * Get time since initialization in milliseconds
 *
 * @return time since initialization
 */
extern uint32_t millis(void);

/**
 * Wait for given amount of time (in ms)
 *
 * Precission is up to 1 ms
 *
 * @param ms    Amount of milliseconds to wait for
 */
extern void delay_ms(uint32_t ms);

/**
 * Initialize timer module (start systick, etc)
 */
extern void Time_Init(void);

#endif

/** @} */
