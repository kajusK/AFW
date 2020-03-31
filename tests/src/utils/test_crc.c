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
 * @file    utils/test_crc.c
 * @brief   Unit tests for crc.c
 *
 * @addtogroup tests
 * @{
 */

#include <main.h>
#include "utils/crc.c"

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(CRC);

TEST_SETUP(CRC)
{
}

TEST_TEAR_DOWN(CRC)
{

}

TEST(CRC, CRC16_Add)
{
    TEST_ASSERT_EQUAL_HEX16(0xe571, CRC16_Add(0xab, CRC16_INITIAL_VALUE));
    TEST_ASSERT_EQUAL_HEX16(0xd46a, CRC16_Add(0xcd, 0xe571));
}

TEST(CRC, CRC16)
{
    uint8_t buf[] = {0xab, 0xcd, 0xef, 0x12};
    TEST_ASSERT_EQUAL_HEX16(0x26f0, CRC16(buf, 4));
}

TEST_GROUP_RUNNER(CRC)
{
    RUN_TEST_CASE(CRC, CRC16_Add);
    RUN_TEST_CASE(CRC, CRC16);
}

void Crc_RunTests(void)
{
    RUN_TEST_GROUP(CRC);
}

/** @} */
