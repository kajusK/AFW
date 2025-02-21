/**
 * @file    modules/uf2.h
 * @brief   USB FW update using UF2 format
 */

#ifndef __MODULES_UF2_H
#define __MODULES_UF2_H

#include <types.h>

/**
 * Process write request of UF2 data block
 *
 * @param block     UF2 block (512 bytes) to be written
 * @return Successfulness of the operation
 */
bool UF2_Write(const uint8_t *data);

/**
 * Read currently running firmware in UF2 data format
 *
 * @param [out] data    Address to store the data to (512 bytes)
 * @param offset        Offset to read in blocks from the first block
 * @return Successfulness of the operation
 */
bool UF2_Read(uint8_t *data, uint32_t offset);

/**
 * Get firmware image size
 *
 * @return Size of the UF2 FW image in bytes
 */
uint32_t UF2_GetImgSize(void);

#endif
