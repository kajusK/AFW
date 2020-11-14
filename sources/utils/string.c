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
 * @file    utils/string.c
 * @brief   Helper functions for working with strings
 *
 * @addtogroup utils
 * @{
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
    for (int8_t i = places-1; i >= 0; i--) {
        buf[i] = dec2hex(value % 16);
        value /= 16;
    }
    buf[places] = '\0';
}

/** @} */
