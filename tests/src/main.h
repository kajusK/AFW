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
 * @file    main.h
 * @brief   Main header for unit tests
 *
 * @addtogroup tests
 * @{
 */

#ifndef __MAIN_H_
#define __MAIN_H_

#include <unity_fixture.h>
#include <types.h>

extern void Time_RunTests(void);
extern void Crc_RunTests(void);
extern void Button_RunTests(void);
extern void Math_RunTests(void);
extern void Nav_RunTests(void);
extern void Ramdisk_RunTests(void);
extern void Nmea_RunTests(void);
extern void Ringbuf_RunTests(void);
extern void Log_RunTests(void);
extern void AES_RunTests(void);
extern void Lora_RunTests(void);
extern void Config_RunTests(void);

extern uint8_t assert_should_fail;

/** assert testing */
#define ASSERT(exp) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wreturn-type\"")  \
    if (assert_should_fail) { \
        TEST_ASSERT_FALSE(exp); \
        return; \
         \
    } else { \
        TEST_ASSERT_TRUE(exp); \
    } \
    _Pragma("GCC diagnostic pop") \

#define ASSERT_NOT(exp) ASSERT(!(exp))

#define extern static

#endif

/** @} */
