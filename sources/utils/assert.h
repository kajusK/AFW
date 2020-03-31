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
 * @file    utils/assert.h
 * @brief   ASSERT macros
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_ASSERT_H
#define __UTILS_ASSERT_H

#include "modules/log.h"

#ifndef ASSERT
    #define ASSERT(condition) \
        if (!(condition)) { \
            Log_Error("ASSERT", "Assertion failed in "__FILE__", line %d", __LINE__); \
            while (1); \
        }
#endif

#ifndef ASSERT_NOT
    #define ASSERT_NOT(condition) ASSERT(!(condition))
#endif

#endif

/** @} */
