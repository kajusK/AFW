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

#include <math.h>
#include "physics.h"

int32_t pressureToAltM(uint32_t pressure_pa, uint32_t sea_level_pa)
{
    /*
     * Based on hypsometric formula, assuming 15 degrees at MSL
     *
     * TODO rewrite with lookup table? Takes 10k of flash because of math.h
     */
    return 44330.0 * (1.0 - pow((float)pressure_pa / sea_level_pa, 0.1902949571836346));
}

/** @} */
