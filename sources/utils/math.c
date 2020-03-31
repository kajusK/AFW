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
 * @file    utils/math.c
 * @brief   Lightweight math functions
 *
 * @addtogroup utils
 * @{
 */

#include "utils/assert.h"
#include "utils/math.h"

/** lookup table for sin*1000 function, 0-90 degrees */
static const uint16_t mathi_sin_lookup[] = {
    0, 17, 35, 52, 70, 87, 105, 122, 139, 156, 174, 191, 208, 225, 242, 259,
    276, 292, 309, 326, 342, 358, 375, 391, 407, 423, 438, 454, 469, 485, 500,
    515, 530, 545, 559, 574, 588, 602, 616, 629, 643, 656, 669, 682, 695, 707,
    719, 731, 743, 755, 766, 777, 788, 799, 809, 819, 829, 839, 848, 857, 866,
    875, 883, 891, 899, 906, 914, 921, 927, 934, 940, 946, 951, 956, 961, 966,
    970, 974, 978, 982, 985, 988, 990, 993, 995, 996, 998, 999, 999, 1000, 1000,
};

/**
 * Find point in sin lookup table and interpolate more
 * precise result
 *
 * @param x     angle in thousands of a degree (up to 90 degrees)
 * @return sin value in thousands
 */
static int32_t mathi_sin_find(int32_t x)
{
    int32_t x1, x2, y1, y2;
    uint8_t pos = x/1000;

    ASSERT_NOT(pos > 90);

    /* point directly in lookup table */
    if (pos * 1000 == x) {
        return (int32_t) mathi_sin_lookup[pos];
    }

    x1 = pos*1000;
    x2 = x1+1000;
    y1 = mathi_sin_lookup[pos];
    y2 = mathi_sin_lookup[pos+1];

    /* similarity of triangles, linear interpolation */
    return ((y2-y1)*(x-x1))/(x2-x1) + y1;
}

int32_t msin(int32_t mdeg)
{
    int32_t sign = 1;

    if (mdeg < 0) {
        mdeg = -mdeg + 180000;
    }

    /* periodic function with period 360 degrees */
    mdeg %= 360000;
    /* 180-360 degrees is same like 0-180, only inverted */
    if (mdeg >= 180000) {
        sign = -1;
        mdeg -= 180000;
    }

    if (mdeg < 90000) {
        return sign*mathi_sin_find(mdeg);
    }
    /* between 90-180, same as (180-angle) */
    return sign*mathi_sin_find(180000-mdeg);
}

int32_t mcos(int32_t mdeg)
{
    return msin(mdeg + 90000);
}

int32_t mtan(int32_t mdeg)
{
    int32_t cos = mcos(mdeg);
    /* avoid division by zero, return huge number */
    if (cos == 0) {
        return ~(1 << 31);
    }
    return (msin(mdeg)*1000)/cos;
}

uint32_t int_sqrt(uint64_t x)
{
    /* based on http://ww1.microchip.com/downloads/en/AppNotes/91040a.pdf */
    /* https://stackoverflow.com/a/10330951 */
    uint32_t res = 0;
    uint32_t add = 1 << 31;
    for(int i = 0; i < 32; i++)
    {
        uint32_t temp = res | add;
        uint64_t g2 = (uint64_t)temp*temp;
        if (x >= g2) {
            res = temp;
        }
        add >>= 1;
    }
    return res;
}

/** @} */
