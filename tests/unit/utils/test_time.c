#include <unity.h>
#include "utils/time.c"

static systickd_cb_t callback;

void Systickd_Init(void)
{
}

void Systickd_SetCallback(systickd_cb_t cb)
{
    callback = cb;
}

void test_millis(void)
{
    Time_Init();

    uint32_t time_prev = millis();
    callback();
    TEST_ASSERT_EQUAL(time_prev + 1, millis());
    TEST_ASSERT_EQUAL(time_prev + 1, millis());
    callback();
    TEST_ASSERT_EQUAL(time_prev + 2, millis());
}
