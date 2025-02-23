#include <unity.h>
#include "utils/crc.c"

void test_CRC16_Add(void)
{
    TEST_ASSERT_EQUAL_HEX16(0xe571, CRC16_Add(0xab, CRC16_INITIAL_VALUE));
    TEST_ASSERT_EQUAL_HEX16(0xd46a, CRC16_Add(0xcd, 0xe571));
}

void test_CRC16(void)
{
    uint8_t buf[] = { 0xab, 0xcd, 0xef, 0x12 };
    TEST_ASSERT_EQUAL_HEX16(0x26f0, CRC16(buf, sizeof(buf)));
}

void test_CRC8_Add(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x0c, CRC8_Add(0xbe, CRC8_INITIAL_VALUE));
    TEST_ASSERT_EQUAL_HEX8(0x92, CRC8_Add(0xef, 0x0c));
}

void test_CRC8(void)
{
    uint8_t buf[] = { 0xbe, 0xef };
    TEST_ASSERT_EQUAL_HEX8(0x92, CRC8(buf, sizeof(buf)));
}
