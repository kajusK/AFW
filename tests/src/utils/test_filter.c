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
 * @file    utils/test_filter.c
 * @brief   Unit tests for filter.c
 *
 * @addtogroup tests
 * @{
 */

#include <main.h>
#include "utils/filter.c"

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(FILTER);

TEST_SETUP(FILTER)
{
}

TEST_TEAR_DOWN(FILTER)
{

}

TEST(FILTER, KalmanSimple)
{
    filter_t filter;
    int32_t res;

    Filter_KalmanSimpleInit(&filter, 1234, 30, 500);
    res = Filter(&filter, 1238);
    TEST_ASSERT_EQUAL(1236, res);
    res = Filter(&filter, 1259);
    TEST_ASSERT_EQUAL(1244, res);
    res = Filter(&filter, 1295);
    TEST_ASSERT_EQUAL(1260, res);
}

TEST_GROUP_RUNNER(FILTER)
{
    RUN_TEST_CASE(FILTER, KalmanSimple);
}

void Filter_RunTests(void)
{
    RUN_TEST_GROUP(FILTER);
}

/** @} */
