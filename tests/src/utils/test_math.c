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
 * @file    utils/test_math.c
 * @brief   Unit tests for math.c
 *
 * @addtogroup tests
 * @{
 */

#include <main.h>
#include "utils/math.c"

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(MATH);

TEST_SETUP(MATH)
{
}

TEST_TEAR_DOWN(MATH)
{

}

TEST(MATH, sin)
{
    TEST_ASSERT_EQUAL(0, msin(0));
    TEST_ASSERT_EQUAL(1000, msin(90000));
    TEST_ASSERT_EQUAL(0, msin(180000));
    TEST_ASSERT_EQUAL(-1000, msin(270000));
    TEST_ASSERT_EQUAL(0, msin(360000));

    TEST_ASSERT_EQUAL(996, msin(85000));
    TEST_ASSERT_EQUAL(276, msin(164000));
    TEST_ASSERT_EQUAL(-438, msin(206000));
    TEST_ASSERT_EQUAL(-292, msin(343000));

    TEST_ASSERT_EQUAL(834, msin(123500));
    TEST_ASSERT_EQUAL(833, msin(123542));

    TEST_ASSERT_EQUAL(833, msin(483542));
}

TEST(MATH, cos)
{
    TEST_ASSERT_EQUAL(1000, mcos(0));
    TEST_ASSERT_EQUAL(0, mcos(90000));
    TEST_ASSERT_EQUAL(-1000, mcos(180000));
    TEST_ASSERT_EQUAL(0, mcos(270000));
    TEST_ASSERT_EQUAL(1000, mcos(360000));

    TEST_ASSERT_EQUAL(551, mcos(8223456));
}

TEST(MATH, tan)
{
    TEST_ASSERT_EQUAL(0, mtan(0));
    TEST_ASSERT_EQUAL(~(1<<31), mtan(90000));
    TEST_ASSERT_EQUAL(0, mtan(180000));
    TEST_ASSERT_EQUAL(~(1<<31), mtan(270000));
    TEST_ASSERT_EQUAL(0, mtan(360000));

    TEST_ASSERT_EQUAL(-482, mtan(1234231));
}

TEST(MATH, sqrt)
{
    TEST_ASSERT_EQUAL(5, int_sqrt(25));
    TEST_ASSERT_EQUAL(10, int_sqrt(100));
    TEST_ASSERT_EQUAL(1234, int_sqrt(1522756));
}

TEST(MATH, ceil_div)
{
    TEST_ASSERT_EQUAL(10, ceil_div(100, 11));
    TEST_ASSERT_EQUAL(4, ceil_div(1234, 341));
    TEST_ASSERT_EQUAL(24, ceil_div(120, 5));
}

TEST_GROUP_RUNNER(MATH)
{
    RUN_TEST_CASE(MATH, sin);
    RUN_TEST_CASE(MATH, cos);
    RUN_TEST_CASE(MATH, tan);
    RUN_TEST_CASE(MATH, sqrt);
    RUN_TEST_CASE(MATH, ceil_div);
}

void Math_RunTests(void)
{
    RUN_TEST_GROUP(MATH);
}

/** @} */
