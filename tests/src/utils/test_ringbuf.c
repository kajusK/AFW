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
 * @file    utils/test_ringbuf.c
 * @brief   Unit tests for ringbuf.c
 *
 * @addtogroup tests
 * @{
 */

#include <string.h>
#include <main.h>
#include "utils/ringbuf.c"

static char buf[4];
static ring_t rbuf;

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(RINGBUF);

TEST_SETUP(RINGBUF)
{
    memset(buf, 0, 4);
    Ring_Init(&rbuf, buf, 4);
}

TEST_TEAR_DOWN(RINGBUF)
{

}

TEST(RINGBUF, buf)
{
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'a'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'b'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'c'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));

    TEST_ASSERT_FALSE(Ring_Push(&rbuf, 'd'));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));

    TEST_ASSERT_EQUAL('a', Ring_Pop(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'e'));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));

    TEST_ASSERT_EQUAL('b', Ring_Pop(&rbuf));
    TEST_ASSERT_EQUAL('c', Ring_Pop(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));
    TEST_ASSERT_FALSE(Ring_Empty(&rbuf));

    TEST_ASSERT_EQUAL('e', Ring_Pop(&rbuf));
    TEST_ASSERT_FALSE(Ring_Full(&rbuf));
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));

    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'f'));
    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'g'));
    TEST_ASSERT_TRUE(Ring_Push(&rbuf, 'h'));
    TEST_ASSERT_TRUE(Ring_Full(&rbuf));
    TEST_ASSERT_EQUAL('f', Ring_Pop(&rbuf));
    TEST_ASSERT_EQUAL('g', Ring_Pop(&rbuf));
    TEST_ASSERT_EQUAL('h', Ring_Pop(&rbuf));
    TEST_ASSERT_TRUE(Ring_Empty(&rbuf));
}

TEST_GROUP_RUNNER(RINGBUF)
{
    RUN_TEST_CASE(RINGBUF, buf);
}

void Ringbuf_RunTests(void)
{
    RUN_TEST_GROUP(RINGBUF);
}

/** @} */
