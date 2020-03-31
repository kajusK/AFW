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
 * @file    modules/log.h
 * @brief   System logging
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_LOG_H
#define __MODULES_LOG_H

#include <types.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
} log_level_t;

/**
 * Set log level
 *
 * @param level     Lowest log level to print
 */
extern void Log_SetLevel(log_level_t level);

/**
 * Raw logging
 *
 * Very limited subset of functionality is supported to reduce overall flash
 * usage. Only bare %%, %d, %u, %x, %s, %c and %p supported in format string
 *
 * @param level     Level of the message to be logged
 * @param source    Source of the message (custom string)
 * @param format    Printf like format
 * @param ...       Printf like arguments
 */
extern void Log_Raw(log_level_t level, const char *source,
        const char *format, ...);

#define Log_Debug(source, format, ...) Log_Raw(LOG_DEBUG, source, format, ##__VA_ARGS__)
#define Log_Info(source, format, ...) Log_Raw(LOG_INFO, source, format, ##__VA_ARGS__)
#define Log_Warning(source, format, ...) Log_Raw(LOG_WARNING, source, format, ##__VA_ARGS__)
#define Log_Error(source, format, ...) Log_Raw(LOG_ERROR, source, format, ##__VA_ARGS__)

#endif

/** @} */
