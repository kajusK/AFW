/**
 * @file    modules/msc.h
 * @brief   USB Mass storage class
 */

#ifndef __MODULES_MSC_H
#define __MODULES_MSC_H

#include <types.h>

/**
 * @param lba - Logical block address - 1 = 512, 2 = 1024,...
 * @param buf - Destination buffer to copy 512 bytes to
 */
typedef int (*msc_read_block_t)(uint32_t lba, uint8_t *buf);
typedef int (*msc_write_block_t)(uint32_t lba, const uint8_t *buf);

/**
 * Initialize the USB Mass Storage
 *
 * @param usbd_dev      The USB device used
 * @param ep_in         Number of the USB IN endpoint
 * @param ep_in_size    The maximum endpoint size (8 to 64)
 * @param ep_out        Number of the USB OUT endpoint
 * @param ep_out_size   The maximum endpoint size (8 to 64)
 * @param vendor_id     The SCSI vendor ID (up to 8 characters)
 * @param product_id    The SCSI product ID (up to 16 characters)
 * @param product_revision_level    The SCSI revision (up to 4 characters)
 * @param read_block    Function to call when host request read of a LBA block
 * @param write_block   Function to call when host request write to a LBA block
 * @param block_count   Amount of 512B blocks available
 */
void Msc_Init(usbd_device *usbd_dev, uint8_t ep_in, uint8_t ep_in_size,
        uint8_t ep_out, uint8_t ep_out_size, const char *vendor_id,
        const char *product_id, const char *product_revision_level,
        msc_read_block_t read_block, msc_write_block_t write_block,
        uint32_t block_count);

#endif
