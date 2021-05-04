/*
 * Copyright (C) 2019 Jakub Kaderka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file    utils/test_string.c
 * @brief   Unit tests for string.c
 *
 * @addtogroup tests
 * @{
 */

#include <ctype.h>
#include <main.h>
#include "utils/string.c"

/* *****************************************************************************
 * Mocks
***************************************************************************** */

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(STRING);

TEST_SETUP(STRING)
{

}

TEST_TEAR_DOWN(STRING)
{

}

TEST(STRING, hex2dec)
{
    for (uint8_t i = 0; i <= 9; i++) {
        TEST_ASSERT_EQUAL_HEX8(i, hex2dec(i+'0'));
    }
    for (uint8_t i = 0; i <= 5; i++) {
        TEST_ASSERT_EQUAL_HEX8(i+10, hex2dec(i+'a'));
        TEST_ASSERT_EQUAL_HEX8(i+10, hex2dec(i+'A'));
    }
    TEST_ASSERT_EQUAL_HEX8(0, hex2dec('w'));
}

TEST(STRING, dec2hex)
{
    for (uint8_t i = 0; i <= 9; i++) {
        TEST_ASSERT_EQUAL('0'+i, dec2hex(i));
    }
    for (uint8_t i = 0; i <= 5; i++) {
        TEST_ASSERT_EQUAL('A'+i, dec2hex(i+10));
    }
    TEST_ASSERT_EQUAL('0', dec2hex(16));
    TEST_ASSERT_EQUAL('0', dec2hex(231));
}

TEST(STRING, num2hex)
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

TEST(STRING, num2str)
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

TEST_GROUP_RUNNER(STRING)
{
    RUN_TEST_CASE(STRING, hex2dec);
    RUN_TEST_CASE(STRING, dec2hex);
    RUN_TEST_CASE(STRING, num2hex);
    RUN_TEST_CASE(STRING, num2str);
}

void String_RunTests(void)
{
    RUN_TEST_GROUP(STRING);
}

/** @} */


