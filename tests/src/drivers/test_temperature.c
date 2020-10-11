/*
 * Copyright (C) 2020 Jakub Kaderka
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
 * @file    modules/test_uf2.c
 * @brief   Unit tests for uf2.c
 *
 * @addtogroup tests
 * @{
 */

#include <string.h>
#include <main.h>
#include "drivers/temperature.c"

/* *****************************************************************************
 * Helpers
***************************************************************************** */
/* *****************************************************************************
 * Mocks
***************************************************************************** */
/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(TEMPERATURE);

TEST_SETUP(TEMPERATURE)
{
}

TEST_TEAR_DOWN(TEMPERATURE)
{
}

TEST(TEMPERATURE, LMT87)
{
    uint32_t delta = 100;
    /* Values from https://www.ti.com/lit/ds/symlink/lmt87.pdf */
    TEST_ASSERT_INT_WITHIN(delta, -50000, LMT87_ConvertmC(3277));
    TEST_ASSERT_INT_WITHIN(delta, 25000, LMT87_ConvertmC(2298));
    TEST_ASSERT_INT_WITHIN(delta, 48000, LMT87_ConvertmC(1985));
    TEST_ASSERT_INT_WITHIN(delta, 143000, LMT87_ConvertmC(640));
    TEST_ASSERT_INT_WITHIN(delta, 150000, LMT87_ConvertmC(538));
}

TEST(TEMPERATURE, TC_K)
{
    /* Max delta from expected value in milli degrees C */
    uint32_t delta = 700;

    TEST_ASSERT_INT_WITHIN(delta, 25000, TC_KConvertmC(0, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 270714, TC_KConvertmC(10000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, -25846, TC_KConvertmC(-2000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 6, TC_KConvertmC(-1000, 25000));

    TEST_ASSERT_INT_WITHIN(delta, 11000, TC_KConvertmC(0, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 256952, TC_KConvertmC(10000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, -40990, TC_KConvertmC(-2000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 133664, TC_KConvertmC(5000, 11000));
}

TEST(TEMPERATURE, TC_J)
{
    /* Max delta from expected value in milli degrees C */
    uint32_t delta = 500;

    TEST_ASSERT_INT_WITHIN(delta, 25000, TC_JConvertmC(0, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 208980, TC_JConvertmC(10000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, -56291, TC_JConvertmC(-4000, 25000));
    TEST_ASSERT_INT_WITHIN(delta, 5486, TC_JConvertmC(-1000, 25000));

    TEST_ASSERT_INT_WITHIN(delta, 11000, TC_JConvertmC(0, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 196018, TC_JConvertmC(10000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, -29186, TC_JConvertmC(-2000, 11000));
    TEST_ASSERT_INT_WITHIN(delta, 105308, TC_JConvertmC(5000, 11000));
}

TEST_GROUP_RUNNER(TEMPERATURE)
{
    RUN_TEST_CASE(TEMPERATURE, LMT87);
    RUN_TEST_CASE(TEMPERATURE, TC_K);
    RUN_TEST_CASE(TEMPERATURE, TC_J);
}

void Temperature_RunTests(void)
{
    RUN_TEST_GROUP(TEMPERATURE);
}

/** @} */
