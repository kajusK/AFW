/**
 * @file    utils/nav.c
 * @brief   Working with lat/lon coordinates - distances, directions,...
 */

#include "utils/math.h"
#include "nav.h"

uint32_t Nav_GetDistanceDm(const nmea_float_t *lat1, const nmea_float_t *lon1,
    const nmea_float_t *lat2, const nmea_float_t *lon2)
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
        mdeg = lat1->num / (lat1->scale / 1000);
    } else {
        mdeg = lat1->num * 1000 / lat1->scale;
    }
    x = abs((lon1->num - lon2->num) * mcos(mdeg)) / 1000;

    /*
     * Simply find distance between two 2D points
     */
    return deglen * int_sqrt((uint64_t)x * x + (uint64_t)y * y) / (lat1->scale / 10);
}

nav_region_t Nav_GetRegion(nmea_float_t latitude, nmea_float_t longitude)
{
    int16_t lat = latitude.num / latitude.scale;
    int16_t lon = longitude.num / longitude.scale;

    if (lat >= 35 && lat <= 73 && lon >= -27 && lon <= 51) {
        return NAV_REGION_EUROPE;
    }
    if (lat >= 10 && lat <= 84 && lon >= -173 && lon <= 33) {
        return NAV_REGION_NORTH_AMERICA;
    }
    if (lat >= -60 && lat <= 14 && lon >= -95 && lon <= -30) {
        return NAV_REGION_SOUTH_AMERICA;
    }
    if (lat >= -37 && lat <= 36 && lon >= -28 && lon <= 61) {
        return NAV_REGION_AFRICA;
    }
    if (lat >= -12 && lat <= 20 && lon >= 90 && lon <= 180) {
        return NAV_REGION_OCEANIA;
    }
    if (lat >= -50 && lat <= -7 && lon >= 108 && lon <= 180) {
        return NAV_REGION_AUSTRALIA_ZEELAND;
    }
    if (lat >= 33 && lat <= 43 && lon >= 124 && lon <= 131) {
        return NAV_REGION_KOREA;
    }
    if (lat >= 4 && lat <= 37 && lon >= 68 && lon <= 92) {
        return NAV_REGION_INDIA;
    }
    if (lat >= 18 && lat <= 55 && lon >= 73 && lon <= 136) {
        return NAV_REGION_CHINA;
    }
    if (lat >= 34 && lat <= 82 && ((lon >= 40 && lon <= 180) || (lon >= -170 && lon <= -180))) {
        return NAV_REGION_ASIA;
    }
    return NAV_REGION_UNKNOWN;
}
