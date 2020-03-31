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
 * @file    utils/test_nav.c
 * @brief   Unit tests for nav.c
 *
 * @addtogroup tests
 * @{
 */

#include <main.h>
#include "utils/math.c"
#include "utils/nav.c"

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(NAV);

TEST_SETUP(NAV)
{
}

TEST_TEAR_DOWN(NAV)
{

}

TEST(NAV, dist)
{
    nmea_float_t lat1 = {17567891, 1000000};
    nmea_float_t lon1 = {23123456, 1000000};
    nmea_float_t lat2 = {17577891, 1000000};
    nmea_float_t lon2 = {23123256, 1000000};

    TEST_ASSERT_EQUAL(11132, Nav_GetDistanceDm(&lat1, &lon1, &lat2, &lon2));

    lat2.num = 17567910;
    lon2.num = 23123446;
    TEST_ASSERT_EQUAL(23, Nav_GetDistanceDm(&lat1, &lon1, &lat2, &lon2));
}

TEST_GROUP_RUNNER(NAV)
{
    RUN_TEST_CASE(NAV, dist);
}

void Nav_RunTests(void)
{
    RUN_TEST_GROUP(NAV);
}

/** @} */
