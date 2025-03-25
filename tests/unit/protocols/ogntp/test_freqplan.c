#include <unity.h>
#include "protocols/ogntp/freqplan.c"

void test_getFreqHz(void)
{
    freq_plan_t plan = { 868200000, 200000, 5 };
    TEST_ASSERT_EQUAL(868600000, getFreqHz(&plan, 2));
}

void test_getHopHash(void)
{
    TEST_ASSERT_EQUAL_HEX32(0x78F11286, getHopHash(12345678));
    TEST_ASSERT_EQUAL_HEX32(0x6D205685, getHopHash(87654321));
}

void test_getChannelEurope(void)
{
    const freq_plan_t *plan = getFreqPlan(NAV_REGION_EUROPE);
    TEST_ASSERT_EQUAL(1, getChannel(plan, 0, 123456));
    TEST_ASSERT_EQUAL(0, getChannel(plan, 1, 654321));
}

void test_getChannelUSA(void)
{
    const freq_plan_t *plan = getFreqPlan(NAV_REGION_NORTH_AMERICA);
    TEST_ASSERT_EQUAL(52, getChannel(plan, 0, 123456));
    TEST_ASSERT_EQUAL(51, getChannel(plan, 1, 123456));
}

void test_getChannelAustralia(void)
{
    const freq_plan_t *plan = getFreqPlan(NAV_REGION_AUSTRALIA_ZEELAND);
    TEST_ASSERT_EQUAL(20, getChannel(plan, 0, 123456));
    TEST_ASSERT_EQUAL(19, getChannel(plan, 1, 123456));
}

void test_getFrequencyHzEurope(void)
{
    TEST_ASSERT_EQUAL(868400000, OGNTP_GetFrequencyHz(NAV_REGION_EUROPE, 0, 123456));
    TEST_ASSERT_EQUAL(868200000, OGNTP_GetFrequencyHz(NAV_REGION_EUROPE, 1, 654321));
}

void test_getFrequencyHzUSA(void)
{
    TEST_ASSERT_EQUAL(923000000, OGNTP_GetFrequencyHz(NAV_REGION_NORTH_AMERICA, 0, 123456));
    TEST_ASSERT_EQUAL(922600000, OGNTP_GetFrequencyHz(NAV_REGION_NORTH_AMERICA, 1, 123456));
}

void test_getFrequencyHzAustralia(void)
{
    TEST_ASSERT_EQUAL(925000000, OGNTP_GetFrequencyHz(NAV_REGION_AUSTRALIA_ZEELAND, 0, 123456));
    TEST_ASSERT_EQUAL(924600000, OGNTP_GetFrequencyHz(NAV_REGION_AUSTRALIA_ZEELAND, 1, 123456));
}
