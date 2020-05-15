/*
 * Copyright (C) 2020 Jakub Kaderka
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
 * @file    modules/test_config.c
 * @brief   Unit tests for config.c
 *
 * @addtogroup tests
 * @{
 */

#include <string.h>
#include <main.h>
#include "modules/config.c"

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(CONFIG);

TEST_SETUP(CONFIG)
{
}

TEST_TEAR_DOWN(CONFIG)
{

}

TEST(CONFIG, SetGetInt)
{
    assert_should_fail = true;
    Config_SetInt(CONFIG_INT_COUNT, 1);
    Config_SetInt(CONFIG_INT_COUNT+10, 1);
    Config_GetInt(CONFIG_INT_COUNT);
    Config_GetInt(CONFIG_INT_COUNT+10);
    assert_should_fail = false;

    /* default values */
    TEST_ASSERT_EQUAL(42, Config_GetInt(CONFIG_INT_TEST1));
    TEST_ASSERT_EQUAL(-1024, Config_GetInt(CONFIG_INT_TEST2));

    Config_SetInt(CONFIG_INT_TEST1, 123);
    Config_SetInt(CONFIG_INT_TEST2, -456);
    TEST_ASSERT_EQUAL(123, Config_GetInt(CONFIG_INT_TEST1));
    TEST_ASSERT_EQUAL(-456, Config_GetInt(CONFIG_INT_TEST2));
}

TEST(CONFIG, SetGetFloat)
{
    assert_should_fail = true;
    Config_SetFloat(CONFIG_FLOAT_COUNT, 1);
    Config_SetFloat(CONFIG_FLOAT_COUNT+10, 1);
    Config_GetFloat(CONFIG_FLOAT_COUNT);
    Config_GetFloat(CONFIG_FLOAT_COUNT+10);
    assert_should_fail = false;

    /* Default values */
    TEST_ASSERT_EQUAL_FLOAT(42.43, Config_GetFloat(CONFIG_FLOAT_TEST1));
    TEST_ASSERT_EQUAL_FLOAT(-123.456, Config_GetFloat(CONFIG_FLOAT_TEST2));

    Config_SetFloat(CONFIG_FLOAT_TEST1, 123.456);
    Config_SetFloat(CONFIG_FLOAT_TEST2, 789.123);
    TEST_ASSERT_EQUAL_FLOAT(123.456, Config_GetFloat(CONFIG_FLOAT_TEST1));
    TEST_ASSERT_EQUAL_FLOAT(789.123, Config_GetFloat(CONFIG_FLOAT_TEST2));
}

TEST(CONFIG, SetGetBool)
{
    assert_should_fail = true;
    Config_SetBool(CONFIG_BOOL_COUNT, 1);
    Config_SetBool(CONFIG_BOOL_COUNT+10, 1);
    Config_GetBool(CONFIG_BOOL_COUNT);
    Config_GetBool(CONFIG_BOOL_COUNT+10);
    assert_should_fail = false;

    /* Default values */
    for (int i = 0; i < CONFIG_BOOL_COUNT; i++) {
        if (i == 1 || i == 10) {
            TEST_ASSERT_TRUE(Config_GetBool(i));
        } else {
            TEST_ASSERT_FALSE(Config_GetBool(i));
        }
    }

    for (int i = 0; i < CONFIG_BOOL_COUNT; i++) {
        Config_SetBool(i, (i*2/3) % 2);
    }
    for (int i = 0; i < CONFIG_BOOL_COUNT; i++) {
        TEST_ASSERT_EQUAL((i*2/3) % 2, Config_GetBool(i));
    }
}

TEST(CONFIG, SetGetString)
{
    char str[] = "Ultra super long string that should be truncated";
    const char *res;

    assert_should_fail = true;
    Config_SetString(CONFIG_STRING_COUNT, "Foo");
    Config_SetString(CONFIG_STRING_COUNT+10, "FOO");
    Config_GetString(CONFIG_STRING_COUNT);
    Config_GetString(CONFIG_STRING_COUNT+10);
    assert_should_fail = false;

    /* Default values */
    TEST_ASSERT_EQUAL_STRING("Hello world", Config_GetString(CONFIG_STRING_TEST1));
    TEST_ASSERT_EQUAL_STRING("Watch the cat!", Config_GetString(CONFIG_STRING_TEST2));

    Config_SetString(CONFIG_STRING_TEST1, "FooBar");
    Config_SetString(CONFIG_STRING_TEST2, str);
    TEST_ASSERT_EQUAL_STRING("FooBar", Config_GetString(CONFIG_STRING_TEST1));

    res = Config_GetString(CONFIG_INT_TEST2);
    TEST_ASSERT_EQUAL_STRING_LEN(str, res, CONFIG_STRING_MAX_LEN);
    TEST_ASSERT_EQUAL('\0', res[CONFIG_STRING_MAX_LEN]);
}

TEST(CONFIG, SetGetPtr)
{
    assert_should_fail = true;
    Config_SetString(CONFIG_PTR_COUNT, "Foo");
    Config_SetString(CONFIG_PTR_COUNT+10, "FOO");
    Config_GetString(CONFIG_PTR_COUNT);
    Config_GetString(CONFIG_PTR_COUNT+10);
    assert_should_fail = false;

    /* Default values */
    TEST_ASSERT_EQUAL_STRING("foo", Config_GetPtr(CONFIG_PTR_TEST1));
    TEST_ASSERT_EQUAL_STRING("bar", Config_GetPtr(CONFIG_PTR_TEST2));

    Config_SetPtr(CONFIG_PTR_TEST1, "FooBar");
    Config_SetPtr(CONFIG_PTR_TEST2, "blah");
    TEST_ASSERT_EQUAL_STRING("FooBar", Config_GetPtr(CONFIG_PTR_TEST1));
    TEST_ASSERT_EQUAL_STRING("blah", Config_GetPtr(CONFIG_PTR_TEST2));
}

TEST_GROUP_RUNNER(CONFIG)
{
    RUN_TEST_CASE(CONFIG, SetGetInt);
    RUN_TEST_CASE(CONFIG, SetGetFloat);
    RUN_TEST_CASE(CONFIG, SetGetBool);
    RUN_TEST_CASE(CONFIG, SetGetString);
    RUN_TEST_CASE(CONFIG, SetGetPtr);
}

void Config_RunTests(void)
{
    RUN_TEST_GROUP(CONFIG);
}

/** @} */


