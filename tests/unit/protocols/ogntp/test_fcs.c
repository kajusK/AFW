#include <unity.h>
#include "utils/utils.c"
#include "protocols/ogntp/fcs.c"

void test_getFCS(void)
{
    uint32_t payload[5] = { 0xaabbccdd, 0xeeff1122, 0x33445566, 0x778899aa, 0x1a2b3c5d };
    static const uint8_t expected_parity[6] = { 0xf8, 0x07, 0x8c, 0x66, 0xa2, 0x15 };
    uint8_t parity[6] = { 0 };

    getFCS((uint8_t *)payload, parity);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_parity, parity, sizeof(expected_parity));
}

void test_isFCSValid(void)
{
    uint32_t payload[5] = { 0xaabbccdd, 0xeeff1122, 0x33445566, 0x778899aa, 0x1a2b3c5d };
    static const uint8_t parity_valid[6] = { 0xf8, 0x07, 0x8c, 0x66, 0xa2, 0x15 };
    static const uint8_t parity_invalid[6] = { 0xf8, 0x07, 0x8c, 0x61, 0xa2, 0x15 };

    TEST_ASSERT_TRUE(isFCSValid((uint8_t *)payload, parity_valid));
    TEST_ASSERT_FALSE(isFCSValid((uint8_t *)payload, parity_invalid));
}
