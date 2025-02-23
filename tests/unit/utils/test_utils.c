#include <unity.h>
#include "utils/utils.c"

void test_count1s(void)
{
    TEST_ASSERT_EQUAL(0, count1s(0));
    TEST_ASSERT_EQUAL(15, count1s(0xabcd1234));
}
