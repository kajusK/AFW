/**
 * @file    fw.c
 * @brief   Firmware upgrade module
 */

#include <types.h>
#include "hal/flash.h"
#include "hal/reloc.h"
#include "utils/crc.h"
#include "fw.h"

/* These symbols are to be defined in a linker file */
extern const char *_fw_runtime_addr;
extern const char *_fw_slot_size;
extern const char *_fw_upgrade_addr;

/** Image Header size, align at 7 bits for VTOR settings */
#define FW_HDR_SIZE     0x80U
/** Size of the firmware slot, including header */
#define FW_SLOT_SIZE    ((uint32_t)&_fw_slot_size)
/** Address of the runtime slot */
#define FW_RUNTIME_ADDR ((uint32_t)&_fw_runtime_addr)
/** Address of the upgrade slot */
#ifdef FW_USE_DUALSLOT
#define FW_UPGRADE_ADDR ((uint32_t)&_fw_upgrade_addr)
#else
#define FW_UPGRADE_ADDR ((uint32_t)&_fw_runtime_addr)
#endif

#ifndef FW_MAGIC
#error "Define FW_MAGIC macro to identify compatible fw images"
#endif

/** FW image header */
typedef struct __attribute__((__packed__)) {
    uint32_t magic; /**< Unique identifier of compatible images */
    uint32_t len;   /**< Length of the firmware image */
    uint16_t crc;   /**< Checksum of the firmware image */
    fw_meta_t meta; /**< Firmware image metadata */
} fw_hdr_t;

/* Header must fit max header size */
_Static_assert(sizeof(fw_hdr_t) == FW_HDR_SIZE, "fw_hdr_t must be FW_HDR_SIZE long");

/** Upgrade progress monitoring */
static struct {
    bool running;         /**< If true, upgrade is in progress */
    uint32_t erase_addr;  /**< First unerased address */
    uint32_t write_addr;  /**< Address to write incoming data to */
    uint32_t written;     /**< Amount of bytes written */
    uint8_t pending_byte; /**< Writes to flash must be 2 byte aligned, pending off byte to write
                             from prev write */
} update_state;

/**
 * Check if the given address contains a valid image (crc, magic,...)
 *
 * @param addr  Address of the image header
 */
static bool isImgValid(uint32_t addr)
{
    const fw_hdr_t *hdr = (const fw_hdr_t *)addr;
    uint16_t crc = 0;

    if (hdr->magic != FW_MAGIC) {
        return false;
    }
    if ((hdr->len + FW_HDR_SIZE) > FW_SLOT_SIZE) {
        return false;
    }

    crc = CRC16((uint8_t *)(addr + FW_HDR_SIZE), hdr->len);
    return hdr->crc == crc;
}

#ifdef FW_USE_DUALSLOT
/**
 * Copy firmware image from one flash address to another one
 *
 * @note Image length is read from image header, destination must be large enough
 *
 * @param target    Address in flash to copy image to
 * @param source    Source address to copy image from
 */
static void copyImage(uint32_t target, uint32_t source)
{
    uint32_t page_size = Flashd_GetPageSize();
    const fw_hdr_t *hdr = (const fw_hdr_t *)source;
    uint32_t end = target + FW_HDR_SIZE + hdr->len;

    Flashd_WriteEnable();
    while (source < end) {
        uint32_t remaining = end - source;
        uint32_t bytes = page_size;
        if (remaining < page_size) {
            bytes = remaining;
        }

        Flashd_ErasePage(target);
        Flashd_Write(target, source, bytes);
        source += bytes;
        target += bytes;
    }
    Flashd_WriteDisable();
}
#endif

bool Fw_Run(void)
{
    bool runtime_valid = isImgValid(FW_RUNTIME_ADDR);
#ifdef FW_USE_DUALSLOT
    const fw_hdr_t *runtime = (const fw_hdr_t *)FW_RUNTIME_ADDR;
    const fw_hdr_t *upgrade = (const fw_hdr_t *)FW_UPGRADE_ADDR;
    bool upgrade_valid = isImgValid(FW_UPGRADE_ADDR);

    if (upgrade_valid && ((runtime->crc != upgrade->crc) || !runtime_valid)) {
        copyImage(FW_RUNTIME_ADDR, FW_UPGRADE_ADDR);
    }
#endif
    if (!runtime_valid) {
        return false;
    }
    Reloc_RunFwBinary(FW_RUNTIME_ADDR + FW_HDR_SIZE);
    return true; // just to make compiler happy
}

bool Fw_UpdateInit(void)
{
    if (update_state.running) {
        return false;
    }
    update_state.erase_addr = FW_UPGRADE_ADDR;
    update_state.write_addr = FW_UPGRADE_ADDR;
    update_state.written = 0;
    update_state.running = true;
    Flashd_WriteEnable();
    return true;
}

bool Fw_Update(const uint8_t *buf, uint32_t len)
{
    if (!update_state.running) {
        return false;
    }
    if ((update_state.write_addr + len) > (FW_UPGRADE_ADDR + FW_SLOT_SIZE)) {
        return false;
    }
    if ((update_state.written == 0) && (len >= sizeof(uint32_t))) {
        // Validate image starts with a valid magic constant
        uint32_t *magic = (uint32_t *)buf;
        if (*magic != FW_MAGIC) {
            return false;
        }
    }

    while (update_state.erase_addr < (update_state.write_addr + len)) {
        Flashd_ErasePage(update_state.erase_addr);
        update_state.erase_addr += Flashd_GetPageSize();
    }

    // There's a pending byte to be written from previous call, 2 byte aligned flash writes
    if ((len != 0) && (update_state.written & 0x1UL)) {
        uint8_t payload[2];
        payload[0] = update_state.pending_byte;
        payload[1] = buf[0];
        Flashd_Write(update_state.write_addr, payload, 2);

        len--;
        buf++;
        update_state.written++;
        update_state.write_addr++;
    }
    if (len & 0x1UL) {
        update_state.pending_byte = buf[len - 1];
        update_state.written++;
        len--;
    }
    Flashd_Write(update_state.write_addr, buf, len);
    update_state.written += len;

    return true;
}

bool Fw_UpdateFinish(void)
{
    if (!update_state.running) {
        return false;
    }
    // 2 byte aligned writes, one pending byte remaining
    if (update_state.written & 0x1UL) {
        Flashd_Write(update_state.write_addr, &update_state.pending_byte, 1);
    }

    update_state.running = false;
    Flashd_WriteDisable();
    return isImgValid(FW_UPGRADE_ADDR);
}

bool Fw_UpdateIsRunning(void)
{
    return update_state.running;
}

bool Fw_IsUpdateNeeded(const uint8_t *buf, uint32_t len)
{
    const fw_hdr_t *update = (const fw_hdr_t *)buf;
    const fw_hdr_t *runtime = (const fw_hdr_t *)FW_RUNTIME_ADDR;

    if (len < (sizeof(fw_hdr_t) - sizeof(fw_meta_t))) {
        // not enough data
        return false;
    }
    if (update->magic != FW_MAGIC) {
        return false;
    }

    if (!isImgValid(FW_RUNTIME_ADDR)) {
        return true;
    }
    return runtime->crc != update->crc;
}

const fw_meta_t *Fw_GetFwMeta(void)
{
    const fw_hdr_t *hdr = (const fw_hdr_t *)FW_RUNTIME_ADDR;
    if (hdr->magic != FW_MAGIC) {
        return NULL;
    }
    return &hdr->meta;
}

const uint8_t *Fw_GetImageAddr(uint32_t *len)
{
    const fw_hdr_t *hdr = (const fw_hdr_t *)FW_RUNTIME_ADDR;
    if (hdr->magic != FW_MAGIC) {
        return NULL;
    }

    if (len != NULL) {
        *len = hdr->len + FW_HDR_SIZE;
    }
    return (uint8_t *)FW_RUNTIME_ADDR;
}
