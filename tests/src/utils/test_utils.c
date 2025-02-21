/*
 * Copyright (C) 2025 Jakub Kaderka
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
 * @file    utils/test_utils.c
 * @brief   Unit tests for utils.c
 *
 * @addtogroup tests
 * @{
 */

#include <main.h>
#include "utils/utils.c"

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(UTILS);

TEST_SETUP(UTILS)
{
}

TEST_TEAR_DOWN(UTILS)
{

}

TEST(UTILS, count1s)
{
    TEST_ASSERT_EQUAL(0, count1s(0));
    TEST_ASSERT_EQUAL(15, count1s(0xabcd1234));
}

TEST_GROUP_RUNNER(UTILS)
{
    RUN_TEST_CASE(UTILS, count1s);
}

void Utils_RunTests(void)
{
    RUN_TEST_GROUP(UTILS);
}

/** @} */
