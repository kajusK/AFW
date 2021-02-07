/*
 * Copyright (C) 2021 Jakub Kaderka
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
 * @file    utils/physics.c
 * @brief   Physics calculations
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_PHYSICS_H
#define __UTILS_PHYSICS_H

#include <types.h>

/** International standard atmosphere pressure at sea level */
#define ISA_SEA_PRESSURE_PA 101325

/**
 * Calculate altitude from athospheric pressure
 *
 * @param presure_pa    Current pressure
 * @param
 */
extern int32_t pressureToAltM(uint32_t pressure_pa, uint32_t sea_level_pa);

#endif

/** @} */
