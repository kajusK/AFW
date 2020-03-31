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
 * @file    utils/math.h
 * @brief   Lightweight math functions
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_MATH_H
#define __UTILS_MATH_H

#include <types.h>

/**
 * Divide number and return smallest integer greater or equal to result
 *
 * @param num   Number to be divided
 * @param div   Divisor
 * @return Result of ceil(num/div)
 */
#define ceil_div(num, div) (((num) + (div) - 1)/(div))

/**
 * Calculate sin function
 *
 * @param mdeg  angle in thousands of a degree
 * @return sin value in thousands
 */
extern int32_t msin(int32_t mdeg);

/**
 * Calculate cos function
 *
 * @param mdeg  angle in thousands of a degree
 * @return cos value in thousands
 */
extern int32_t mcos(int32_t mdeg);

/**
 * Calculate tan function
 *
 * @param mdeg  angle in thousands of a degree
 * @return tan value in thousands
 */
extern int32_t mtan(int32_t mdeg);

/**
 * Calculate square root with integer arithmetics only
 *
 * @param x     Number to get square root of
 * @return  Square root of the number
 */
extern uint32_t int_sqrt(uint64_t x);

#endif

/** @} */
