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
 * @file    modules/config.c
 * @brief   System configuration handling
 *
 * The data are stored at the end of the flash memory in two separate partitions,
 * each partition has a CRC at the beginning. Two copies are for fault tolerance
 * when the writing is aborted during the process.
 *
 * The first partition is written and read first, second is the fallback one.
 * The linker must define _config_part1 and 2, both in different flash pages.
 * Both flash pages gets erased. The config_t should fit within one page.
 *
 *
 * @addtogroup modules
 * @{
 */

#include <string.h>
#include "hal/flash.h"
#include "utils/crc.h"
#include "utils/assert.h"
#include "config.h"

/** Structure of the configuration stored in flash memory */
typedef struct {
    uint16_t crc;       /** Checksum of the config data */
    config_t config;
} config_internal_t;

extern const config_internal_t _config_part1;
#if CONFIG_USE_TWO_PARTITIONS
    extern const config_internal_t _config_part2;
#endif

static const config_t *conf_valid;

const config_t *Config_Get(void)
{
    return conf_valid;
}

bool Config_Read(void)
{
    if (CRC16((uint8_t *)&_config_part1.config, sizeof(config_t)) == _config_part1.crc) {
        conf_valid = &_config_part1.config;
#if CONFIG_USE_TWO_PARTITIONS
    } else if (CRC16((uint8_t *)&_config_part2.config, sizeof(config_t)) == _config_part2.crc) {
        conf_valid = &_config_part2.config;
#endif
    } else {
        return false;
    }
    return true;
}

void Config_Write(const config_t *config)
{
    ASSERT_NOT(config == NULL);
    config_internal_t loc_config;

    memcpy(&loc_config.config, config, sizeof(config_t));
    loc_config.crc = CRC16((uint8_t *)config, sizeof(config_t));

    Flashd_WriteEnable();

    if (_config_part1.crc != loc_config.crc) {
        Flashd_ErasePage((uint32_t)&_config_part1);
        Flashd_Write((uint32_t)&_config_part1, (uint8_t *)&loc_config, sizeof(config_internal_t));
    }
#if CONFIG_USE_TWO_PARTITIONS
    if (_config_part2.crc != loc_config.crc) {
        Flashd_ErasePage((uint32_t)&_config_part2);
        Flashd_Write((uint32_t)&_config_part2, (uint8_t *)&loc_config, sizeof(config_internal_t));
    }
#endif
    Flashd_WriteDisable();

    conf_valid = &_config_part1.config;
}

/** @} */
