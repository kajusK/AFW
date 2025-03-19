#include <unity.h>
#include "utils/math.c"
#include "utils/nav.c"

void test_Nav_GetDistanceDm(void)
{
    nmea_float_t lat1 = { 17567891, 1000000 };
    nmea_float_t lon1 = { 23123456, 1000000 };
    nmea_float_t lat2 = { 17577891, 1000000 };
    nmea_float_t lon2 = { 23123256, 1000000 };

    TEST_ASSERT_EQUAL(11132, Nav_GetDistanceDm(&lat1, &lon1, &lat2, &lon2));

    lat2.num = 17567910;
    lon2.num = 23123446;
    TEST_ASSERT_EQUAL(23, Nav_GetDistanceDm(&lat1, &lon1, &lat2, &lon2));
}

void test_Nav_GetRegion(void)
{
    nav_region_t region;

    region = Nav_GetRegion((nmea_float_t){ 502, 10 }, (nmea_float_t){ 196, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_EUROPE, region);

    region = Nav_GetRegion((nmea_float_t){ 66123, 1000 }, (nmea_float_t){ 9732, 100 });
    TEST_ASSERT_EQUAL(NAV_REGION_ASIA, region);

    region = Nav_GetRegion((nmea_float_t){ 57123, 1000 }, (nmea_float_t){ -1091, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_NORTH_AMERICA, region);

    region = Nav_GetRegion((nmea_float_t){ -512, 10 }, (nmea_float_t){ -701, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_SOUTH_AMERICA, region);

    region = Nav_GetRegion((nmea_float_t){ -81, 10 }, (nmea_float_t){ 241, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_AFRICA, region);

    region = Nav_GetRegion((nmea_float_t){ -72, 10 }, (nmea_float_t){ 1429, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_OCEANIA, region);

    region = Nav_GetRegion((nmea_float_t){ -192, 10 }, (nmea_float_t){ 1440, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_AUSTRALIA_ZEELAND, region);

    region = Nav_GetRegion((nmea_float_t){ 366, 10 }, (nmea_float_t){ 1277, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_KOREA, region);

    region = Nav_GetRegion((nmea_float_t){ 517, 10 }, (nmea_float_t){ 1236, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_CHINA, region);

    region = Nav_GetRegion((nmea_float_t){ 223, 10 }, (nmea_float_t){ 778, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_INDIA, region);

    region = Nav_GetRegion((nmea_float_t){ -271, 10 }, (nmea_float_t){ -1323, 10 });
    TEST_ASSERT_EQUAL(NAV_REGION_UNKNOWN, region);
}
