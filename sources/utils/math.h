/**
 * @file    utils/math.h
 * @brief   Lightweight math functions
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
int32_t msin(int32_t mdeg);

/**
 * Calculate cos function
 *
 * @param mdeg  angle in thousands of a degree
 * @return cos value in thousands
 */
int32_t mcos(int32_t mdeg);

/**
 * Calculate tan function
 *
 * @param mdeg  angle in thousands of a degree
 * @return tan value in thousands
 */
int32_t mtan(int32_t mdeg);

/**
 * Calculate square root with integer arithmetics only
 *
 * @param x     Number to get square root of
 * @return  Square root of the number
 */
uint32_t int_sqrt(uint64_t x);

#endif
