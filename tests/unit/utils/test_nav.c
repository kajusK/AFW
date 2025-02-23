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
