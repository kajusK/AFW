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
 * @file    modules/config.h
 * @brief   System configuration handling
 *
 * A config_items.h file is required for this to work, the file can be generated
 * with tools/config_items.py from templates/config_items.ods. Example file can
 * be found in tests/src/config_items.h
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_CONFIG_H
#define __MODULES_CONFIG_H

#include <types.h>
#include "config_items.h"

#ifdef CONFIG_INT_DEFAULTS
/**
 * Get integer config value
 *
 * @param [in] item ID of item requested
 * @return requested item value
 */
extern int32_t Config_GetInt(config_item_int_t item);

/**
 * Set integer item config value
 *
 * @param [in] item ID of item requested
 * @param [in] value value to store
 */
extern void Config_SetInt(config_item_int_t item, int32_t value);
#endif

#ifdef CONFIG_FLOAT_DEFAULTS
/**
 * Get float item config value
 *
 * @param [in] item ID of item requested
 * @return requested item value
 */
extern float Config_GetFloat(config_item_float_t item);

/**
 * Set float item config value
 *
 * @param [in] item ID of item requested
 * @param [in] value value to store
 */
extern void Config_SetFloat(config_item_float_t item, float value);
#endif

#ifdef CONFIG_BOOL_DEFAULTS
/**
 * Get bool item config value
 *
 * @param [in] item ID of item requested
 * @return requested item value
 */
extern bool Config_GetBool(config_item_bool_t item);

/**
 * Set bool item config value
 *
 * @param [in] item ID of item requested
 * @param [in] value value to store
 */
extern void Config_SetBool(config_item_bool_t item, bool value);
#endif

#ifdef CONFIG_STRING_DEFAULTS
/**
 * Get string item config value
 *
 * @param [in] item ID of item requested
 * @return requested item value
 */
extern const char *Config_GetString(config_item_string_t item);

/**
 * Set string item config value
 *
 * @param [in] item ID of item requested
 * @param [in] value value to store
 */
extern void Config_SetString(config_item_string_t item, const char *value);
#endif

#ifdef CONFIG_PTR_DEFAULTS
/**
 * Get string pointer item config value
 *
 * @param [in] item ID of item requested
 * @return requested item value
 */
extern const char *Config_GetPtr(config_item_ptr_t item);

/**
 * Set string pointer item config value
 *
 * @param [in] item ID of item requested
 * @param [in] value value to store
 */
extern void Config_SetPtr(config_item_ptr_t item, const char *value);
#endif

#endif
/** @} */
