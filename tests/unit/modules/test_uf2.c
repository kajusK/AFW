#include <unity.h>
#include <string.h>
#include "modules/uf2.c"

struct {
    bool running;
    uint8_t data[2048];
    uint32_t last_len;
    uint32_t len;
} test_fw;

bool Fw_Update(const uint8_t *buf, uint32_t len)
{
    if (!test_fw.running) {
        return false;
    }
    if (test_fw.len + len > sizeof(test_fw.data)) {
        return false;
    }
    memcpy(&test_fw.data[test_fw.len], buf, len);
    test_fw.last_len = len;
    test_fw.len += len;
    return true;
}

bool Fw_UpdateIsRunning(void)
{
    return test_fw.running;
}

bool Fw_UpdateInit(void)
{
    test_fw.running = true;
    test_fw.len = 0;
    return true;
}

bool Fw_UpdateFinish(void)
{
    test_fw.running = false;
    return true;
}

const uint8_t *Fw_GetImageAddr(uint32_t *len)
{
    if (len != NULL) {
        *len = test_fw.len;
    }
    return test_fw.data;
}

void test_Write(void)
{
    UF2_block_t block;

    block.magicStart0 = UF2_MAGIC_1;
    block.magicStart1 = UF2_MAGIC_2;
    block.flags = 0;
    block.targetAddr = 0;
    block.payloadSize = UF2_CHUNK_SIZE;
    block.numBlocks = 2;
    block.fileSize = 123;
    block.magicEnd = UF2_MAGIC_FINAL;

    /* FW update first chunk */
    block.blockNo = 0;
    for (unsigned i = 0; i < UF2_CHUNK_SIZE; i++) {
        block.data[i] = i;
    }
    TEST_ASSERT_TRUE(UF2_Write((uint8_t *)&block));
    TEST_ASSERT_TRUE(test_fw.running);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, test_fw.len);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, test_fw.last_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(block.data, test_fw.data, test_fw.len);

    /* FW update final chunk */
    block.payloadSize = 64;
    block.blockNo = 1;
    block.targetAddr = UF2_CHUNK_SIZE;
    for (unsigned i = 0; i < block.payloadSize; i++) {
        block.data[i] = i * 2;
    }
    TEST_ASSERT_TRUE(UF2_Write((uint8_t *)&block));
    TEST_ASSERT_FALSE(test_fw.running);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE + 64, test_fw.len);
    TEST_ASSERT_EQUAL(block.payloadSize, test_fw.last_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(block.data, &test_fw.data[UF2_CHUNK_SIZE], 64);
}

void test_Read(void)
{
    UF2_block_t block;

    test_fw.len = UF2_CHUNK_SIZE + 64;
    for (unsigned i = 0; i < test_fw.len; i++) {
        test_fw.data[i] = (uint8_t)i;
    }

    TEST_ASSERT_TRUE(UF2_Read((uint8_t *)&block, 0));
    TEST_ASSERT_EQUAL(0, block.flags);
    TEST_ASSERT_EQUAL(0, block.blockNo);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, block.payloadSize);
    TEST_ASSERT_EQUAL(0, block.targetAddr);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(test_fw.data, block.data, block.payloadSize);

    TEST_ASSERT_TRUE(UF2_Read((uint8_t *)&block, 1));
    TEST_ASSERT_EQUAL(0, block.flags);
    TEST_ASSERT_EQUAL(1, block.blockNo);
    TEST_ASSERT_EQUAL(64, block.payloadSize);
    TEST_ASSERT_EQUAL(UF2_CHUNK_SIZE, block.targetAddr);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&test_fw.data[UF2_CHUNK_SIZE], block.data, block.payloadSize);
}

void test_GetImgSize(void)
{
    test_fw.len = UF2_CHUNK_SIZE * 2;
    TEST_ASSERT_EQUAL(2 * 512, UF2_GetImgSize());

    test_fw.len = UF2_CHUNK_SIZE * 2 - 1;
    TEST_ASSERT_EQUAL(2 * 512, UF2_GetImgSize());

    test_fw.len = UF2_CHUNK_SIZE + 1;
    TEST_ASSERT_EQUAL(2 * 512, UF2_GetImgSize());

    test_fw.len = UF2_CHUNK_SIZE;
    TEST_ASSERT_EQUAL(1 * 512, UF2_GetImgSize());
}
