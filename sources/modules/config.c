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
 * @file    modules/config.c
 * @brief   System configuration handling
 *
 * @addtogroup modules
 * @{
 */

#include <string.h>
#include "utils/assert.h"
#include "config.h"

#ifdef CONFIG_INT_DEFAULTS
static int config_item_int[CONFIG_INT_COUNT] = CONFIG_INT_DEFAULTS;

int32_t Config_GetInt(config_item_int_t item)
{
    ASSERT_NOT(item >= CONFIG_INT_COUNT);
    return config_item_int[item];
}

void Config_SetInt(config_item_int_t item, int32_t value)
{
    ASSERT_NOT(item >= CONFIG_INT_COUNT);
    config_item_int[item] = value;
}
#endif

#ifdef CONFIG_FLOAT_DEFAULTS
static float config_item_float[CONFIG_FLOAT_COUNT] = CONFIG_FLOAT_DEFAULTS;

float Config_GetFloat(config_item_float_t item)
{
    ASSERT_NOT(item >= CONFIG_FLOAT_COUNT);
    return config_item_float[item];
}

void Config_SetFloat(config_item_float_t item, float value)
{
    ASSERT_NOT(item >= CONFIG_FLOAT_COUNT);
    config_item_float[item] = value;
}
#endif

#ifdef CONFIG_BOOL_DEFAULTS
static uint8_t config_item_bool[CONFIG_BOOL_COUNT/8+1] = CONFIG_BOOL_DEFAULTS;

bool Config_GetBool(config_item_bool_t item)
{
    uint8_t mask;
    ASSERT_NOT(item >= CONFIG_BOOL_COUNT);

    mask = 1 << (item % 8);
    return config_item_bool[item / 8] & mask;
}

void Config_SetBool(config_item_bool_t item, bool value)
{
    uint8_t mask;
    ASSERT_NOT(item >= CONFIG_BOOL_COUNT);

    mask = 1 << (item % 8);
    if (value) {
        config_item_bool[item / 8] |= mask;
    } else {
        config_item_bool[item / 8] &= ~mask;
    }
}
#endif

#ifdef CONFIG_STRING_DEFAULTS
static char config_item_string[CONFIG_STRING_COUNT][CONFIG_STRING_MAX_LEN+1] =
        CONFIG_STRING_DEFAULTS;

const char *Config_GetString(config_item_string_t item)
{
    ASSERT_NOT(item >= CONFIG_STRING_COUNT);
    return config_item_string[item];
}

void Config_SetString(config_item_string_t item, const char *value)
{
    ASSERT_NOT(item >= CONFIG_STRING_COUNT);
    uint32_t max_len = sizeof(config_item_string[item]);

    strncpy(config_item_string[item], value, max_len - 1);
    config_item_string[item][max_len-1] = '\0';
}
#endif

#ifdef CONFIG_PTR_DEFAULTS
static const char *config_item_ptr[CONFIG_PTR_COUNT] = CONFIG_PTR_DEFAULTS;

const char *Config_GetPtr(config_item_ptr_t item)
{
    ASSERT_NOT(item >= CONFIG_PTR_COUNT);
    return config_item_ptr[item];
}

void Config_SetPtr(config_item_ptr_t item, const char *value)
{
    ASSERT_NOT(item >= CONFIG_PTR_COUNT);
    config_item_ptr[item] = value;
}

#endif

/** @} */
