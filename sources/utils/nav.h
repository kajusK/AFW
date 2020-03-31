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
 * @file    utils/nav.h
 * @brief   Working with lat/lon coordinates - distances, directions,...
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_NAV_H
#define __UTILS_NAV_H

#include <types.h>
#include "modules/nmea.h"

/**
 * Calculate distance between two gps points
 *
 * Note this function works only for short distances, larger distances
 * than few kms are causing overflow during calculations.
 *
 * For precise measurements, there's a theory around great-circle distance,
 * usually calculated by Haversine formula which is quite overkill for MCUs.
 * On shorter distances, Earth can be considered flat, therefore euclidean
 * distance should be sufficient in range up to several km (error below
 * 1 %).
 *
 * @param lat1  Latitude of point 1
 * @param lon1  Longitude of point1
 * @param lat2  Latitude of point 2
 * @param lon2  Longitude of point2
 *
 * @return Distance in decimeters
 */
extern uint32_t Nav_GetDistanceDm(nmea_float_t *lat1, nmea_float_t *lon1,
        nmea_float_t *lat2, nmea_float_t *lon2);

#endif

/** @} */
