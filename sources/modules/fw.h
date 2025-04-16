/**
 * @file    modules/fw.h
 * @brief   Firmware upgrade module
 *
 * There are two supported memory layouts:
 *  - One firmware slot. FW must be updated by bootloader only, no fallback - runtime slot rewritten
 * during update
 *  - Two slots. FW can be update by bootloader or from firmware itself.
 *    - First slot contains a runtime binary
 *    - Second slot contains a fw upgrade - rewritten during update
 *    - If a valid upgrade image is found, it replaces the runtime slot (no need for position
 * independent code)
 *
 * For this module to work:
 *   - Define FW_MAGIC - uint32_t unique identifier of compatible fw image
 *   - Define linker symbol _fw_runtime_addr = address of the runtime img area.
 *   - Define linker symbol _fw_slot_size = size of the runtime slot
 *   - If two slot arrangement is needed
 *      - define FW_USE_DUALSLOT (e.g. in makefile)
 *      - define linker symbol _fw_upgrade_addr
 *      - runtime slot must be at least as long as the upgrade one
 *
 * Example linker header for bootloader (for single slot, drop upgrade section)
 *
 *     MEMORY
 *     {
 *         rom (rx) : ORIGIN = 0x08000000, LENGTH = 4K
 *         runtime (rx) : ORIGIN = 0x08001000, LENGTH = 14K
 *         upgrade (rx) : ORIGIN = 0x08004800, LENGTH = 14K
 *         ram (rwx) : ORIGIN = 0x20000000, LENGTH = 6K
 *     }
 *
 *     _fw_runtime_addr = ORIGIN(runtime);
 *     _fw_slot_size = LENGTH(runtime);
 *     _fw_upgrade_addr = ORIGIN(upgrade);
 *
 * And for firmware
 *
 *     MEMORY
 *     {
 *         rom (rx) : ORIGIN = 0x08001080, LENGTH = 14K
 *         upgrade (rx) : ORIGIN = 0x08004800, LENGTH = 14K
 *         ram (rwx) : ORIGIN = 0x20000000, LENGTH = 6K
 *     }
 *
 *     _fw_runtime_addr = ORIGIN(rom)-0x80;
 *     _fw_slot_size = LENGTH(rom);
 *     _fw_upgrade_addr = ORIGIN(upgrade);
 *
 */

#ifndef __MODULES_FW_H
#define __MODULES_FW_H

#include <types.h>

/** Metadata of the firmware/bootloader image */
typedef struct __attribute__((__packed__)) {
    uint8_t major;     /**< Major FW version, 0 for devel */
    uint8_t minor;     /**< Minor FW version, 0 for devel */
    uint8_t patch;     /**< Patch FW version, 0 for devel */
    char git_hash[47]; /**< Git has from which this fw was built 40 B + 6B dirty + trailing zero */
    char description[68]; /**< Textual description of the binary */
} fw_meta_t;

/**
 * Run firmware (to be used from bootloader)
 *
 * @return False if no valid image found. Otherwise never returns.
 */
bool Fw_Run(void);

/**
 * Initialize FW update
 *
 * @return True if succeeded, false if already running
 */
bool Fw_UpdateInit(void);

/**
 * Write chunk of the update data
 *
 * @note The fw image data shall start with proper header (see sources)
 *
 * @param buf       Data to be written
 * @param len       Length of the data
 *
 * @return True if succeeded, False otherwise
 */
bool Fw_Update(const uint8_t *buf, uint32_t len);

/**
 * Finish the FW update - check final CRC, lock flash,...
 *
 * @note Shall be called even if the Fw_Update call fails
 *
 * @return true if succeeded (crc matches,...)
 */
bool Fw_UpdateFinish(void);

/**
 * Get state of the FW update
 */
bool Fw_UpdateIsRunning(void);

/**
 * Check if the firmware needs to be upgraded to given image
 *
 * @param buf   Initial bytes of the update image (at least 10)
 * @param len   Length of the buffer
 * @return True if runtime img is not present or differs from the update data
 */
bool Fw_IsUpdateNeeded(const uint8_t *buf, uint32_t len);

/**
 * Get FW image metadata
 *
 * @return metadata or NULL if image is not present.
 */
const fw_meta_t *Fw_GetFwMeta(void);

/**
 * Get memory address of the runtime image (including header)
 *
 * @param len   If not NULL, store image length here
 */
const uint8_t *Fw_GetImageAddr(uint32_t *len);

#endif
