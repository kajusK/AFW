#include <unity.h>
#include <math.h>
#include "utils/math.c"

void test_sin(void)
{
    TEST_ASSERT_EQUAL(0, msin(0));
    TEST_ASSERT_EQUAL(1000, msin(90000));
    TEST_ASSERT_EQUAL(0, msin(180000));
    TEST_ASSERT_EQUAL(-1000, msin(270000));
    TEST_ASSERT_EQUAL(0, msin(360000));

    TEST_ASSERT_EQUAL(996, msin(85000));
    TEST_ASSERT_EQUAL(276, msin(164000));
    TEST_ASSERT_EQUAL(-438, msin(206000));
    TEST_ASSERT_EQUAL(-292, msin(343000));

    TEST_ASSERT_EQUAL(834, msin(123500));
    TEST_ASSERT_EQUAL(833, msin(123542));

    TEST_ASSERT_EQUAL(833, msin(483542));
    TEST_ASSERT_EQUAL(-833, msin(-483542));
}

void test_cos(void)
{
    TEST_ASSERT_EQUAL(1000, mcos(0));
    TEST_ASSERT_EQUAL(0, mcos(90000));
    TEST_ASSERT_EQUAL(-1000, mcos(180000));
    TEST_ASSERT_EQUAL(0, mcos(270000));
    TEST_ASSERT_EQUAL(1000, mcos(360000));

    TEST_ASSERT_EQUAL(551, mcos(8223456));
}

void test_tan(void)
{
    TEST_ASSERT_EQUAL(0, mtan(0));
    TEST_ASSERT_EQUAL(~(1 << 31), mtan(90000));
    TEST_ASSERT_EQUAL(0, mtan(180000));
    TEST_ASSERT_EQUAL(~(1 << 31), mtan(270000));
    TEST_ASSERT_EQUAL(0, mtan(360000));

    TEST_ASSERT_EQUAL(-482, mtan(1234231));
}

void test_sqrt(void)
{
    TEST_ASSERT_EQUAL(5, int_sqrt(25));
    TEST_ASSERT_EQUAL(10, int_sqrt(100));
    TEST_ASSERT_EQUAL(1234, int_sqrt(1522756));
}

void test_ceil_div(void)
{
    TEST_ASSERT_EQUAL(10, ceil_div(100, 11));
    TEST_ASSERT_EQUAL(4, ceil_div(1234, 341));
    TEST_ASSERT_EQUAL(24, ceil_div(120, 5));
}
