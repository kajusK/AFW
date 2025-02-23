#include <unity.h>
#include "drivers/temperature.c"

void test_temperature_lmt87(void)
{
    uint32_t delta = 100;
    /* Values from https://www.ti.com/lit/ds/symlink/lmt87.pdf */
    TEST_ASSERT_INT_WITHIN(delta, -50000, LMT87_ConvertmC(3277));
    TEST_ASSERT_INT_WITHIN(delta, 25000, LMT87_ConvertmC(2298));
    TEST_ASSERT_INT_WITHIN(delta, 48000, LMT87_ConvertmC(1985));
    TEST_ASSERT_INT_WITHIN(delta, 143000, LMT87_ConvertmC(640));
    TEST_ASSERT_INT_WITHIN(delta, 150000, LMT87_ConvertmC(538));
}

void test_temperature_tc_k(void)
{
    /* Max delta from expected value in milli degrees C */
    uint32_t delta = 700;

    TEST_ASSERT_INT_WITHIN(delta, 25000, TC_KConvertmC(0, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 270714, TC_KConvertmC(10000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, -25846, TC_KConvertmC(-2000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 6, TC_KConvertmC(-1000, 25000));

    TEST_ASSERT_INT_WITHIN(delta, 11000, TC_KConvertmC(0, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 256952, TC_KConvertmC(10000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, -40990, TC_KConvertmC(-2000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 133664, TC_KConvertmC(5000, 11000));
}

void test_temperature_tc_j(void)
{
    /* Max delta from expected value in milli degrees C */
    uint32_t delta = 500;

    TEST_ASSERT_INT_WITHIN(delta, 25000, TC_JConvertmC(0, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 208980, TC_JConvertmC(10000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, -56291, TC_JConvertmC(-4000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 5486, TC_JConvertmC(-1000, 25000));

    TEST_ASSERT_INT_WITHIN(delta, 11000, TC_JConvertmC(0, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 196018, TC_JConvertmC(10000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, -29186, TC_JConvertmC(-2000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 105308, TC_JConvertmC(5000, 11000));
}
