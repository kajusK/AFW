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
 * @file    modules/test_log.c
 * @brief   Unit tests for log.c
 *
 * @addtogroup tests
 * @{
 */

#include <stdarg.h>
#include <string.h>
#include <main.h>
#include "modules/log.c"

char uart_output[200];
int uart_pos = 0;

/* *****************************************************************************
 * Helpers
***************************************************************************** */
static void test_print(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Logi_Printf(fmt, ap);
    va_end(ap);
}

/* *****************************************************************************
 * Mocks
***************************************************************************** */
uint32_t millis(void)
{
    return 123;
}

void UARTd_Puts(uint8_t device, const char *msg)
{
    TEST_ASSERT_EQUAL(123, device);
    strcat(uart_output, msg);
    uart_pos += strlen(msg);
}

void UARTd_Putc(uint8_t device, char c)
{
    TEST_ASSERT_EQUAL(123, device);
    uart_output[uart_pos] = c;
    uart_output[uart_pos + 1] = '\0';
    uart_pos += 1;
}

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(LOG);

TEST_SETUP(LOG)
{
    uart_pos = 0;
    uart_output[0] = '\0';
    Log_Init(123);
}

TEST_TEAR_DOWN(LOG)
{

}

TEST(LOG, Uitoa)
{
    Logi_Uitoa(0, 10);
    TEST_ASSERT_EQUAL_STRING("0", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(1, 10);
    TEST_ASSERT_EQUAL_STRING("1", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(10, 10);
    TEST_ASSERT_EQUAL_STRING("10", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(109, 10);
    TEST_ASSERT_EQUAL_STRING("109", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(123456789, 10);
    TEST_ASSERT_EQUAL_STRING("123456789", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa((uint32_t)-1, 10);
    TEST_ASSERT_EQUAL_STRING("4294967295", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(0xa, 16);
    TEST_ASSERT_EQUAL_STRING("A", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(0xf, 16);
    TEST_ASSERT_EQUAL_STRING("F", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(0x10, 16);
    TEST_ASSERT_EQUAL_STRING("10", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa(0xabcdef12, 16);
    TEST_ASSERT_EQUAL_STRING("ABCDEF12", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;

    Logi_Uitoa((uint32_t)-1, 16);
    TEST_ASSERT_EQUAL_STRING("FFFFFFFF", uart_output);
    uart_output[0] = '\0';
    uart_pos = 0;
}

TEST(LOG, Printf)
{
    test_print("foo %% %s, %c, %d %u 0x%x %p", "bar", 'a',
            -123, 23212321, 0xabc2, (uint32_t *)0xabcd);
    TEST_ASSERT_EQUAL_STRING("foo % bar, a, -123 23212321 0xABC2 0xABCD",
            uart_output);
}

TEST(LOG, Raw)
{
    Log_SetLevel(LOG_INFO);
    Log_Raw(LOG_DEBUG, "FOO", "foo %d", 2);
    TEST_ASSERT_EQUAL('\0', uart_output[0]);

    Log_Raw(LOG_INFO, "FOO", "foo %d", 2);
    TEST_ASSERT_EQUAL_STRING("[123] "TERM_NORMAL"INFO: "TERM_NORMAL"FOO - foo 2\r\n",
            uart_output);
}

TEST_GROUP_RUNNER(LOG)
{
    RUN_TEST_CASE(LOG, Uitoa);
    RUN_TEST_CASE(LOG, Printf);
    RUN_TEST_CASE(LOG, Raw);
}

void Log_RunTests(void)
{
    RUN_TEST_GROUP(LOG);
}

/** @} */


