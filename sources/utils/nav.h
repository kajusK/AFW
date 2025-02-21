/**
 * @file    utils/nav.h
 * @brief   Working with lat/lon coordinates - distances, directions,...
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
uint32_t Nav_GetDistanceDm(const nmea_float_t *lat1,
        const nmea_float_t *lon1, const nmea_float_t *lat2,
        const nmea_float_t *lon2);

#endif
