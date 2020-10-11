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
#include "modules/uf2.c"

struct {
    bool running;
    uint16_t crc;
    uint32_t len;
    uint8_t data[2048];
    uint32_t last_addr;
    uint32_t last_len;
} test_fw;

/* *****************************************************************************
 * Helpers
***************************************************************************** */
/* *****************************************************************************
 * Mocks
***************************************************************************** */
static bool Fw_Update(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    if (!test_fw.running) {
        return false;
    }

    if (addr + len > sizeof(test_fw.data)) {
        return false;
    }
    memcpy(&test_fw.data[addr], buf, len);
    test_fw.last_addr = addr;
    test_fw.last_len = len;
    return true;
}

static bool Fw_UpdateIsRunning(void)
{
    return test_fw.running;
}

static bool Fw_UpdateInit(uint16_t crc, uint32_t len)
{
    test_fw.crc = crc;
    test_fw.len = len;
    test_fw.running = true;
    return true;
}

static bool Fw_UpdateFinish(void)
{
    test_fw.running = false;
    return true;
}

static uint8_t *Fw_GetCurrent(uint32_t *length, uint32_t *crc)
{
    if (length != NULL) {
        *length = test_fw.len;
    }
    if (crc != NULL) {
        *crc = test_fw.crc;
    }
    return test_fw.data;
}

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(UF2);

TEST_SETUP(UF2)
{
    memset(&test_fw, 0, sizeof(test_fw));
}

TEST_TEAR_DOWN(UF2)
{
}

TEST(UF2, Write)
{
    UF2_block_t block;
    UF2_fw_header_t header;

    header.crc = 0xdead;
    header.len = 0x00b00b1e;

    block.magicStart0 = UF2_MAGIC_1;
    block.magicStart1 = UF2_MAGIC_2;
    block.flags = UF2_FLAG_NOT_MAIN_FLASH;
    block.targetAddr = 0;
    block.payloadSize = sizeof(header);
    block.blockNo = 0;
    block.numBlocks = 3;
    block.fileSize = 123;
    block.magicEnd = UF2_MAGIC_FINAL;
    memcpy(block.data, &header, sizeof(header));

    /* FW update initialize */
    TEST_ASSERT_TRUE(UF2_Write((uint8_t *)&block));
    TEST_ASSERT_TRUE(test_fw.running);
    TEST_ASSERT_EQUAL_HEX(header.crc, test_fw.crc);
    TEST_ASSERT_EQUAL_HEX(header.len, test_fw.len);
    TEST_ASSERT_EQUAL(0, test_fw.last_addr);
    TEST_ASSERT_EQUAL(0, test_fw.last_len);

    /* FW update first chunk */
    block.flags = 0;
    block.payloadSize = UF2_CHUNK_SIZE;
    block.blockNo = 1;
    for (unsigned i = 0; i < UF2_CHUNK_SIZE; i++) {
        block.data[i] = i;
    }
    TEST_ASSERT_TRUE(UF2_Write((uint8_t *)&block));
    TEST_ASSERT_TRUE(test_fw.running);
    TEST_ASSERT_EQUAL(0, test_fw.last_addr);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, test_fw.last_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(block.data, test_fw.data, test_fw.last_len);

    /* FW update final chunk */
    block.payloadSize = 64;
    block.blockNo = 2;
    block.targetAddr = UF2_CHUNK_SIZE;
    for (unsigned i = 0; i < block.payloadSize; i++) {
        block.data[i] = i*2;
    }
    TEST_ASSERT_TRUE(UF2_Write((uint8_t *)&block));
    TEST_ASSERT_FALSE(test_fw.running);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, test_fw.last_addr);
    TEST_ASSERT_EQUAL(block.payloadSize, test_fw.last_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(block.data, &test_fw.data[test_fw.last_addr],
            test_fw.last_len);
}

TEST(UF2, Read)
{
    UF2_block_t block;
    UF2_fw_header_t *header;

    test_fw.len = UF2_CHUNK_SIZE+64;
    test_fw.crc = 0xdead;
    for (unsigned i = 0; i < test_fw.len; i++) {
        test_fw.data[i] = (uint8_t) i;
    }

    TEST_ASSERT_TRUE(UF2_Read((uint8_t *) &block, 0));
    header = (UF2_fw_header_t *)block.data;
    TEST_ASSERT_EQUAL_HEX32(UF2_MAGIC_1, block.magicStart0);
    TEST_ASSERT_EQUAL_HEX32(UF2_MAGIC_2, block.magicStart1);
    TEST_ASSERT_EQUAL_HEX32(UF2_MAGIC_FINAL, block.magicEnd);
    TEST_ASSERT_EQUAL(test_fw.len, block.fileSize);
    TEST_ASSERT_EQUAL(3, block.numBlocks);
    TEST_ASSERT_EQUAL(UF2_FLAG_NOT_MAIN_FLASH, block.flags);
    TEST_ASSERT_EQUAL(0, block.blockNo);
    TEST_ASSERT_EQUAL(6, block.payloadSize);
    TEST_ASSERT_EQUAL(0, block.targetAddr);
    TEST_ASSERT_EQUAL(test_fw.len, header->len);
    TEST_ASSERT_EQUAL_HEX16(test_fw.crc, header->crc);

    TEST_ASSERT_TRUE(UF2_Read((uint8_t *) &block, 1));
    TEST_ASSERT_EQUAL(0, block.flags);
    TEST_ASSERT_EQUAL(1, block.blockNo);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, block.payloadSize);
    TEST_ASSERT_EQUAL(0, block.targetAddr);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(test_fw.data, block.data, block.payloadSize);

    TEST_ASSERT_TRUE(UF2_Read((uint8_t *) &block, 2));
    TEST_ASSERT_EQUAL(0, block.flags);
    TEST_ASSERT_EQUAL(2, block.blockNo);
    TEST_ASSERT_EQUAL(64, block.payloadSize);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, block.targetAddr);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&test_fw.data[UF2_CHUNK_SIZE], block.data,
            block.payloadSize);
}

TEST(UF2, GetImgSize)
{
    test_fw.len = UF2_CHUNK_SIZE*2;
    TEST_ASSERT_EQUAL(3*512, UF2_GetImgSize());

    test_fw.len = UF2_CHUNK_SIZE*2-1;
    TEST_ASSERT_EQUAL(3*512, UF2_GetImgSize());

    test_fw.len = UF2_CHUNK_SIZE+1;
    TEST_ASSERT_EQUAL(3*512, UF2_GetImgSize());

    test_fw.len = UF2_CHUNK_SIZE;
    TEST_ASSERT_EQUAL(2*512, UF2_GetImgSize());
}

TEST_GROUP_RUNNER(UF2)
{
    RUN_TEST_CASE(UF2, Write);
    RUN_TEST_CASE(UF2, Read);
    RUN_TEST_CASE(UF2, GetImgSize);
}

void UF2_RunTests(void)
{
    RUN_TEST_GROUP(UF2);
}

/** @} */
