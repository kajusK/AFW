/**
 * @file    modules/fw.h
 * @brief   Firmware upgrade module
 */

#ifndef __MODULES_FW_H
#define __MODULES_FW_H

#include <types.h>

/**
 * Find latest valid image and boot it
 */
void Fw_Run(void);

/**
 * Reboot the MCU
 */
void Fw_Reboot(void);

/**
 * Initialize FW update and erase FW area
 *
 * @param major     Major fw version
 * @param minor     Minor fw version
 * @param crc       Expected CRC of the image
 * @param len       Length of the image
 *
 * @return True if succeeded
 */
bool Fw_UpdateInit(uint16_t crc, uint32_t len);

/**
 * Write update data to selected address
 *
 * @param addr      Address in the image area to write data to
 * @param buf       Data to be written
 * @param len       Length of the data
 *
 * @return True if succeeded, False otherwise (update aborted due to error)
 */
bool Fw_Update(uint32_t addr, const uint8_t *buf, uint32_t len);

/**
 * Abort the FW update process
 */
void Fw_UpdateAbort(void);

/**
 * Finish the FW update - check final CRC, write headers,...
 *
 * @return true if suceeded (crc matches,...)
 */
bool Fw_UpdateFinish(void);

/**
 * Get state of the FW update
 *
 * @return True if running, false otherwise
 */
bool Fw_UpdateIsRunning(void);

/**
 * Get current FW image
 *
 * @param [out] length  NULL or address to store image length
 * @param [out] crc     NULL or address to store image crc
 *
 * @return Pointer to the image image start
 */
uint8_t *Fw_GetCurrent(uint32_t *length, uint32_t *crc);

#endif
