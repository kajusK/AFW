/**
 * @file    modules/uf2.c
 * @brief   USB FW update using UF2 format
 *
 * https://github.com/Microsoft/uf2
 */

#include <string.h>
#include <modules/fw.h>
#include "uf2.h"

#define UF2_MAGIC_1 0x0A324655
#define UF2_MAGIC_2 0x9E5D5157
#define UF2_MAGIC_FINAL 0x0AB16F30

#define UF2_FLAG_NOT_MAIN_FLASH 0x00000001
#define UF2_FLAG_FILE_CONTAINER 0x00001000
#define UF2_FLAG_FAMILY_ID_PRESENT 0x00002000
#define UF2_FLAG_MD5_CHECKSUM 0x00004000

#define UF2_CHUNK_SIZE 256

/** Structure of the UF2 block */
typedef struct {
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t fileSize;
    uint8_t data[476];
    uint32_t magicEnd;
} UF2_block_t;

typedef struct {
    uint16_t crc;
    uint32_t len;
} __attribute__((packed)) UF2_fw_header_t;

bool UF2_Write(const uint8_t *data)
{
    UF2_block_t *block = (UF2_block_t *)data;
    UF2_fw_header_t *header;

    if (block->magicStart0 != 0x0A324655 || block->magicStart1 != 0x9E5D5157 ||
            block->magicEnd != 0x0AB16F30) {
        return false;
    }
    /* First block should be 'not main flash' with 2 bytes crc and 4 bytes length */
    if (block->flags & UF2_FLAG_NOT_MAIN_FLASH && !Fw_UpdateIsRunning() &&
            block->blockNo == 0 && block->payloadSize == 6) {
        header = (UF2_fw_header_t *)block->data;
        Fw_UpdateInit(header->crc, header->len);
        return true;
    }
    if (block->flags & (UF2_FLAG_NOT_MAIN_FLASH | UF2_FLAG_FILE_CONTAINER)) {
        return true;
    }
    if (!Fw_UpdateIsRunning()) {
        return false;
    }

    Fw_Update(block->targetAddr, block->data, block->payloadSize);
    /* let's assume we are sending block in correct order */
    if (block->blockNo == block->numBlocks - 1) {
        Fw_UpdateFinish();
    }
    return true;
}

bool UF2_Read(uint8_t *data, uint32_t offset)
{
    uint32_t len, crc;
    UF2_block_t *block = (UF2_block_t *)data;
    UF2_fw_header_t *header;
    uint8_t *p = Fw_GetCurrent(&len, &crc);
    uint32_t addr = (offset - 1) * UF2_CHUNK_SIZE;

    if (offset != 0 && addr > len) {
        return false;
    }

    memset(data, 0xff, 512);
    block->magicStart0 = 0x0A324655;
    block->magicStart1 = 0x9E5D5157;
    block->magicEnd = 0x0AB16F30;
    block->fileSize = len;
    /* apply ceil, add 1 for non flash block with header */
    block->numBlocks = (len + UF2_CHUNK_SIZE - 1)/UF2_CHUNK_SIZE + 1;
    block->flags = 0;
    block->blockNo = offset;

    if (offset == 0) {
        block->flags = UF2_FLAG_NOT_MAIN_FLASH;
        block->payloadSize = 6;
        block->targetAddr = 0;
        header = (UF2_fw_header_t *)block->data;
        header->crc = crc;
        header->len = len;
    } else {
        block->targetAddr = addr;
        block->payloadSize = len - (offset - 1)*UF2_CHUNK_SIZE;
        if (block->payloadSize > UF2_CHUNK_SIZE) {
            block->payloadSize = UF2_CHUNK_SIZE;
        }
        memcpy(block->data, p + addr, block->payloadSize);
    }
    return true;
}

uint32_t UF2_GetImgSize(void)
{
    uint32_t len;
    (void) Fw_GetCurrent(&len, NULL);
    return ((len + UF2_CHUNK_SIZE - 1)/UF2_CHUNK_SIZE + 1)*512;
}
