/**
 * @file    modules/log.c
 * @brief   System logging
 */

#include <stdarg.h>
#include <string.h>
#include "hal/io.h"
#include "hal/uart.h"
#include "utils/time.h"

#include "log.h"

#define TERM_NORMAL  "\x1B[0m"
#define TERM_RED     "\x1B[31m"
#define TERM_GREEN   "\x1B[32m"
#define TERM_YELLOW  "\x1B[33m"
#define TERM_BLUE    "\x1B[34m"
#define TERM_MAGENTA "\x1B[35m"
#define TERM_CYAN    "\x1B[36m"
#define TERM_WHITE   "\x1B[37m"

static log_level_t logi_level = LOG_INFO;
static uint8_t logi_uart = 0xff;

/**
 * Convert number to string and send to serial
 *
 * @param num   Number to be converted
 * @param base  Base of the number (10 for decimal, 16 for hex,...)
 */
static void Logi_Uitoa(uint32_t num, uint8_t base)
{
    char buf[11];
    char *start = buf;
    char *pos = buf;
    char tmp;

    do {
        *pos = num % base;
        if (*pos >= 10) {
            *pos += 'A' - 10;
        } else {
            *pos += '0';
        }
        num /= base;
        pos++;
    } while (num != 0 && pos < (buf + sizeof(buf) - 1));
    *pos = '\0';
    pos--;

    /** Reverse string */
    while (start < pos) {
        tmp = *start;
        *start++ = *pos;
        *pos-- = tmp;
    }

    UARTd_Puts(logi_uart, buf);
}

/**
 * Printf like function to print to serial
 *
 * Very limited subset of functionality is supported to reduce overall flash
 * usage. Only bare %%, %d, %u, %x, %s, %c and %p supported
 *
 * @param fmt       Format string in printf like form
 * @param ap        va_list already initialized by caller
 */
static void Logi_Printf(const char *fmt, va_list ap)
{
    int num;
    uint32_t unum;

    while (*fmt) {
        if (*fmt != '%') {
            UARTd_Putc(logi_uart, *fmt);
            fmt++;
            continue;
        }
        fmt++;

        switch (*fmt) {
            case '%':
                UARTd_Putc(logi_uart, '%');
                break;
            case 'c':
                UARTd_Putc(logi_uart, (char)va_arg(ap, int));
                break;
            case 's':
                UARTd_Puts(logi_uart, va_arg(ap, const char *));
                break;
            case 'p':
                UARTd_Puts(logi_uart, "0x");
                Logi_Uitoa((uint32_t)va_arg(ap, uint32_t *), 16);
                break;
            case 'd':
            case 'i':
                num = va_arg(ap, int);
                if (num < 0) {
                    UARTd_Putc(logi_uart, '-');
                    num = -num;
                }
                Logi_Uitoa((uint32_t)num, 10);
                break;
            case 'u':
                unum = va_arg(ap, unsigned int);
                Logi_Uitoa(unum, 10);
                break;
            case 'X':
            case 'x':
                unum = va_arg(ap, unsigned int);
                Logi_Uitoa(unum, 16);
                break;
            case '\0':
                return;
            default:
                break;
        }
        fmt++;
    }
}

void Log_SetLevel(log_level_t level)
{
    logi_level = level;
}

void Log_AddLine(log_level_t level, const char *source, const char *format, ...)
{
    va_list ap;

    if (level < logi_level) {
        return;
    }
    if (logi_uart == 0xff) {
        return;
    }

    UARTd_Putc(logi_uart, '[');
    Logi_Uitoa(millis(), 10);
    UARTd_Puts(logi_uart, "] ");

    switch (level) {
        case LOG_DEBUG:
            UARTd_Puts(logi_uart, TERM_WHITE "DEBUG: ");
            break;
        case LOG_INFO:
            UARTd_Puts(logi_uart, TERM_NORMAL "INFO: ");
            break;
        case LOG_WARNING:
            UARTd_Puts(logi_uart, TERM_YELLOW "WARN: ");
            break;
        case LOG_ERROR:
            UARTd_Puts(logi_uart, TERM_RED "ERROR: ");
            break;
    }
    UARTd_Puts(logi_uart, TERM_NORMAL);

    if (source != NULL && strlen(source) != 0) {
        UARTd_Puts(logi_uart, source);
        UARTd_Puts(logi_uart, " - ");
    }

    va_start(ap, format);
    Logi_Printf(format, ap);
    va_end(ap);
    UARTd_Puts(logi_uart, "\r\n");
}

void Log_Raw(const char *str)
{
    UARTd_Puts(logi_uart, str);
}

void Log_RawChar(char c)
{
    UARTd_Putc(logi_uart, c);
}

void Log_Init(uint8_t uart_device)
{
    logi_uart = uart_device;
}
