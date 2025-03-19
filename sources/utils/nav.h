/**
 * @file    utils/nav.h
 * @brief   Working with lat/lon coordinates - distances, directions,...
 */

#ifndef __UTILS_NAV_H
#define __UTILS_NAV_H

#include <types.h>
#include "modules/nmea.h"

/** World regions */
typedef enum {
    NAV_REGION_EUROPE,
    NAV_REGION_ASIA,
    NAV_REGION_NORTH_AMERICA,
    NAV_REGION_SOUTH_AMERICA,
    NAV_REGION_AFRICA,
    NAV_REGION_OCEANIA,
    NAV_REGION_AUSTRALIA_ZEELAND,
    NAV_REGION_KOREA,
    NAV_REGION_CHINA,
    NAV_REGION_INDIA,
    NAV_REGION_UNKNOWN,
} nav_region_t;

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
uint32_t Nav_GetDistanceDm(const nmea_float_t *lat1, const nmea_float_t *lon1,
    const nmea_float_t *lat2, const nmea_float_t *lon2);

/**
 * Estimate the world region we are in
 *
 * Very rough estimation from rectangular boxes drawn on map
 */
nav_region_t Nav_GetRegion(nmea_float_t latitude, nmea_float_t longitude);

#endif
