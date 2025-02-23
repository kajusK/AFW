#include <unity.h>
#include "utils/string.c"

void test_hex2dec(void)
{
    for (uint8_t i = 0; i <= 9; i++) {
        TEST_ASSERT_EQUAL_HEX8(i, hex2dec(i + '0'));
    }
    for (uint8_t i = 0; i <= 5; i++) {
        TEST_ASSERT_EQUAL_HEX8(i + 10, hex2dec(i + 'a'));
        TEST_ASSERT_EQUAL_HEX8(i + 10, hex2dec(i + 'A'));
    }
    TEST_ASSERT_EQUAL_HEX8(0, hex2dec('w'));
}

void test_dec2hex(void)
{
    for (uint8_t i = 0; i <= 9; i++) {
        TEST_ASSERT_EQUAL('0' + i, dec2hex(i));
    }
    for (uint8_t i = 0; i <= 5; i++) {
        TEST_ASSERT_EQUAL('A' + i, dec2hex(i + 10));
    }
    TEST_ASSERT_EQUAL('0', dec2hex(16));
    TEST_ASSERT_EQUAL('0', dec2hex(231));
}

void test_num2hex(void)
{
    char buf[17];

    num2hex(0x12, 0, buf);
    TEST_ASSERT_EQUAL_STRING("", buf);
    num2hex(0x12, 2, buf);
    TEST_ASSERT_EQUAL_STRING("12", buf);
    num2hex(0x12, 4, buf);
    TEST_ASSERT_EQUAL_STRING("0012", buf);
    num2hex(0x12, 8, buf);
    TEST_ASSERT_EQUAL_STRING("00000012", buf);

    num2hex(0x1234abcd, 0, buf);
    TEST_ASSERT_EQUAL_STRING("", buf);
    num2hex(0x1234abcd, 2, buf);
    TEST_ASSERT_EQUAL_STRING("CD", buf);
    num2hex(0x1234abcd, 4, buf);
    TEST_ASSERT_EQUAL_STRING("ABCD", buf);
    num2hex(0x1234abcd, 8, buf);
    TEST_ASSERT_EQUAL_STRING("1234ABCD", buf);

    num2hex(0x1234abcd, 3, buf);
    TEST_ASSERT_EQUAL_STRING("BCD", buf);

    num2hex(0x1234abcd, 9, buf);
    TEST_ASSERT_EQUAL_STRING("", buf);
}

void test_num2str(void)
{
    char buf[17];

    num2str(123, buf, 1);
    TEST_ASSERT_EQUAL_STRING("", buf);
    num2str(123, buf, 4);
    TEST_ASSERT_EQUAL_STRING("123", buf);
    num2str(123, buf, 2);
    TEST_ASSERT_EQUAL_STRING("1", buf);
    num2str(123, buf, 10);
    TEST_ASSERT_EQUAL_STRING("123", buf);

    num2str(123456789, buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("123456789", buf);
}
