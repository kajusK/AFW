/**
 * @file    utils/string.c
 * @brief   Helper functions for working with strings
 */

#include <ctype.h>
#include "utils/string.h"

uint8_t hex2dec(char c)
{
    if (!isxdigit(c)) {
        return 0;
    }
    if (isdigit(c)) {
        return c - '0';
    }
    return tolower(c) - 'a' + 10;
}

char dec2hex(uint8_t num)
{
    if (num > 15) {
        return '0';
    }
    if (num > 9) {
        return num + 'A' - 10;
    }
    return num + '0';
}

void num2hex(uint32_t value, uint8_t places, char *buf)
{
    if (places > 8) {
        *buf = '\0';
        return;
    }
    for (int8_t i = places - 1; i >= 0; i--) {
        buf[i] = dec2hex(value % 16);
        value /= 16;
    }
    buf[places] = '\0';
}

void num2str(uint32_t value, char *buf, size_t len)
{
    uint32_t mult = 1;
    while (mult < value / 10) {
        mult *= 10;
    }

    while (mult != 0 && len > 1) {
        *buf++ = value / mult + '0';
        value %= mult;
        mult /= 10;
        len--;
    }
    if (len) {
        *buf = '\0';
        buf--;
    }
}
