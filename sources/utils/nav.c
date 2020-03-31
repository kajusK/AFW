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
 * @file    utils/nav.c
 * @brief   Working with lat/lon coordinates - distances, directions,...
 *
 * @addtogroup utils
 * @{
 */

#include "utils/math.h"
#include "nav.h"

uint32_t Nav_GetDistanceDm(nmea_float_t *lat1, nmea_float_t *lon1,
        nmea_float_t *lat2, nmea_float_t *lon2)
{
    int32_t x, y, mdeg;

    /*
     * Let's get x and y differences first
     *
     * Longitude (east-west position) difference depends on latitude,
     * closer to pole, shorter the longitude circle (circle with constant
     * latitude). On equator, the length is pi*d (d is Earth diameter). On
     * pole, the length is zero.
     *
     * Latitude circle is same length everywhere, it's length is pi*d
     */

    /* m per 1 degree or latitude circle (2*pi*Equatorial radius)/360 */
    const uint32_t deglen = 111317;

    /* Latitude first */
    y = abs(lat1->num - lat2->num);
    /* Longitude second, cos(lat) times smaller than on equator */
    if (lat1->scale >= 1000) {
        mdeg = lat1->num / (lat1->scale/1000);
    } else {
        mdeg = lat1->num * 1000/lat1->scale;
    }
    x = abs((lon1->num - lon2->num)*mcos(mdeg))/1000;

    /*
     * Simply find distance between two 2D points
     */
    return deglen*int_sqrt((uint64_t)x*x + (uint64_t)y*y)/(lat1->scale/10);
}

/** @} */
