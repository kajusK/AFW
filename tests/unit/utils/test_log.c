#include <stdarg.h>
#include <string.h>
#include <unity.h>
#include "modules/log.c"

char uart_output[200];
int uart_pos = 0;

static void test_print(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    Logi_Printf(fmt, ap);
    va_end(ap);
}

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

void setUp(void)
{
    uart_pos = 0;
    uart_output[0] = '\0';
    Log_Init(123);
}

void test_Uitoa(void)
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

void test_Printf(void)
{
    test_print("foo %% %s, %c, %d %u 0x%x %p", "bar", 'a', -123, 23212321, 0xabc2,
        (uint32_t *)0xabcd);
    TEST_ASSERT_EQUAL_STRING("foo % bar, a, -123 23212321 0xABC2 0xABCD", uart_output);
}

void test_Raw(void)
{
    Log_SetLevel(LOG_INFO);
    Log_Raw(LOG_DEBUG, "FOO", "foo %d", 2);
    TEST_ASSERT_EQUAL('\0', uart_output[0]);

    Log_Raw(LOG_INFO, "FOO", "foo %d", 2);
    TEST_ASSERT_EQUAL_STRING("[123] " TERM_NORMAL "INFO: " TERM_NORMAL "FOO - foo 2\r\n",
        uart_output);
}
