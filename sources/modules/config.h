/*
 * Copyright (C) 2021 Jakub Kaderka
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
 * A config_struct.h file is required for this to work, the file has to contain
 * a config_t type definition, this config type will be stored to the flash.
 *
 * Additionally two linker symbols in flash memory must be defined for two
 * config partitions: _config_part1 and _config_part2. Each partition should
 * be placed in a separate flash page.
 *
 * Optionally the CONFIG_USE_TWO_PARITIONS can be defined to 0 to avoid
 * using the _config_part2 partition, in such cases, the config is stored
 * in a single partition.
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_CONFIG_H
#define __MODULES_CONFIG_H

#include <types.h>
#include "config_struct.h"

#ifndef CONFIG_USE_TWO_PARTITIONS
    #define CONFIG_USE_TWO_PARTITIONS 1
#endif

/**
 * Get the configuration
 *
 * @return Pointer to config or null if no valid config found
 */
extern const config_t *Config_Get(void);

/**
 * Read the stored configuration
 *
 * @return True if succeeded, false if no valid config found (in this case,
 *          the caller should provide default values and write them to storage)
 */
extern bool Config_Read(void);

/**
 * Write the configuration to memory
 *
 * @param config        Config data to write
 */
extern void Config_Write(const config_t *config);

#endif

/** @} */
