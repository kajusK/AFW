#include <unity.h>
#include "protocols/encoding/manchester.c"

void test_ManchesterEncode_single_byte(void)
{
    uint8_t input[] = { 0x8d };
    uint8_t expected[] = { 0x6a, 0x59 };
    uint8_t output[sizeof(expected)];

    ManchesterEncode(output, input, sizeof(input));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, output, sizeof(input) * 2);
}

void test_ManchesterEncode_multiple_bytes(void)
{
    uint8_t input[] = { 0x9c, 0x90, 0x9b, 0x9a };
    uint8_t expected[] = { 0x69, 0x5A, 0x69, 0xAA, 0x69, 0x65, 0x69, 0x66 };
    uint8_t output[sizeof(expected)];

    ManchesterEncode(output, input, sizeof(input));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, output, sizeof(input) * 2);
}

void test_ManchesterDecode_single_byte(void)
{
    uint8_t input[] = { 0x6a };
    uint8_t output[2];

    bool result = ManchesterDecode(output, input, sizeof(input));
    TEST_ASSERT_FALSE(result);
}

void test_ManchesterDecode_two_bytes(void)
{
    uint8_t input[] = { 0x6a, 0x59 };
    uint8_t expected[] = { 0x8d };
    uint8_t output[sizeof(expected)];

    bool result = ManchesterDecode(output, input, sizeof(input));
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, output, sizeof(input) / 2);
}

void test_ManchesterDecode_multiple_bytes(void)
{
    uint8_t input[] = { 0x69, 0x5A, 0x69, 0xAA, 0x69, 0x65, 0x69, 0x66 };
    uint8_t expected[] = { 0x9c, 0x90, 0x9b, 0x9a };
    uint8_t output[sizeof(expected)];

    bool result = ManchesterDecode(output, input, sizeof(input));
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, output, sizeof(input) / 2);
}

void test_ManchesterDecode_invalid_data(void)
{
    uint8_t input[] = { 0xAA, 0xAB };
    uint8_t output[1];

    bool result = ManchesterDecode(output, input, sizeof(input));
    TEST_ASSERT_FALSE(result);
}

void test_ManchesterEncodeDecode(void)
{
    const uint32_t input[] = { 0x56565555, 0xa5aa5959, 0xa56aa656, 0x966aa55a, 0xa56959a9,
        0x65655699, 0x6a6a6a5a, 0x665a59a5, 0xa555a955, 0x5a956566, 0xa5959956, 0x9966959a,
        0x66956a66 };
    const uint32_t encoded[(sizeof(input) * 2) / 4];
    const uint32_t decoded[(sizeof(input)) / 4];

    ManchesterEncode((uint8_t *)encoded, (uint8_t *)input, sizeof(input));
    TEST_ASSERT_TRUE(ManchesterDecode((uint8_t *)decoded, (uint8_t *)encoded, sizeof(input) * 2));
    TEST_ASSERT_EQUAL_HEX32_ARRAY(input, decoded, sizeof(input) / 4);
}
