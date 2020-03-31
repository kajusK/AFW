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
 * @file    hal/wdg.h
 * @brief   Watchdog driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_WDG_H
#define __HAL_WDG_H

#include <types.h>

/**
 * Initialize watchdog
 *
 * @param period_ms     Watchdog timer period in millisecond
 */
extern void Wdgd_Init(uint32_t period_ms);

/**
 * Clear watchdog timer
 *
 * Once initialized, this function must be called periodically to avoid
 * WDG mcu reset
 */
extern void Wdgd_Clear(void);

#endif

/** @} */
