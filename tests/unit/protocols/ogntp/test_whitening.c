#include <unity.h>
#include "protocols/ogntp/whitening.c"

void test_whitenPayload(void)
{
    uint32_t payload[] = { 0xaabbccdd, 0xeeff1122, 0x33445566, 0x778899aa };
    uint32_t expected[] = { 0x2bb9207b, 0x96a85668, 0x78e0a795, 0x0ffb8812 };

    whitenPayload((uint8_t *)payload);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, payload, 4);
}

void test_dewhitenPayload(void)
{
    uint32_t payload[] = { 0x2bb9207b, 0x96a85668, 0x78e0a795, 0x0ffb8812 };
    uint32_t expected[] = { 0xaabbccdd, 0xeeff1122, 0x33445566, 0x778899aa };

    dewhitenPayload((uint8_t *)payload);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(expected, payload, 4);
}
