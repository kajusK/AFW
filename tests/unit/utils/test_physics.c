#include <math.h>
#include <unity.h>
#include "utils/physics.c"

void test_pressureToAltM(void)
{
    TEST_ASSERT_EQUAL(7, pressureToAltM(101234, 101325));
    TEST_ASSERT_EQUAL(988, pressureToAltM(90000, 101325));
    TEST_ASSERT_EQUAL(2308, pressureToAltM(76500, 101325));

    TEST_ASSERT_EQUAL(98, pressureToAltM(97561, 98712));
    TEST_ASSERT_EQUAL(832, pressureToAltM(89351, 98712));
}
