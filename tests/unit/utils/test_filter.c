#include <unity.h>
#include "utils/filter.c"

void test_kalmanSimple(void)
{
    filter_t filter;
    int32_t res;

    Filter_KalmanSimpleInit(&filter, 1234, 30, 500);
    res = Filter(&filter, 1238);
    TEST_ASSERT_EQUAL(1236, res);
    res = Filter(&filter, 1259);
    TEST_ASSERT_EQUAL(1244, res);
    res = Filter(&filter, 1295);
    TEST_ASSERT_EQUAL(1260, res);
}
