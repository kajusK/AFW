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
 * @file    utils/test_time.c
 * @brief   Unit tests for time.c
 *
 * @addtogroup tests
 * @{
 */

#include <main.h>
#include "utils/time.c"

/* *****************************************************************************
 * Mocks
***************************************************************************** */
static systickd_cb_t callback;

void Systickd_Init(void)
{

}

void Systickd_SetCallback(systickd_cb_t cb)
{
    callback = cb;
}

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(TIME);

TEST_SETUP(TIME)
{
}

TEST_TEAR_DOWN(TIME)
{

}

TEST(TIME, millis)
{
    Time_Init();

    TEST_ASSERT_NOT_NULL(callback);

    uint32_t time_prev = millis();
    callback();
    TEST_ASSERT_EQUAL(time_prev + 1, millis());
    TEST_ASSERT_EQUAL(time_prev + 1, millis());
    callback();
    TEST_ASSERT_EQUAL(time_prev + 2, millis());
}

TEST_GROUP_RUNNER(TIME)
{
    RUN_TEST_CASE(TIME, millis);
}

void Time_RunTests(void)
{
    RUN_TEST_GROUP(TIME);
}

/** @} */


