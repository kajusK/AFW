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
 * @file    utils/test_physics.c
 * @brief   Unit tests for physics.c
 *
 * @addtogroup tests
 * @{
 */

#include <ctype.h>
#include <main.h>
#include "utils/physics.c"

/* *****************************************************************************
 * Mocks
***************************************************************************** */

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(PHYSICS);

TEST_SETUP(PHYSICS)
{

}

TEST_TEAR_DOWN(PHYSICS)
{

}

TEST(PHYSICS, pressureToAltM)
{
    TEST_ASSERT_EQUAL(7, pressureToAltM(101234, 101325));
    TEST_ASSERT_EQUAL(988, pressureToAltM(90000, 101325));
    TEST_ASSERT_EQUAL(2308, pressureToAltM(76500, 101325));

    TEST_ASSERT_EQUAL(98, pressureToAltM(97561, 98712));
    TEST_ASSERT_EQUAL(832, pressureToAltM(89351, 98712));
}

TEST_GROUP_RUNNER(PHYSICS)
{
    RUN_TEST_CASE(PHYSICS, pressureToAltM);
}

void Physics_RunTests(void)
{
    RUN_TEST_GROUP(PHYSICS);
}
