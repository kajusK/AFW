/**
 * @file    modules/fw.c
 * @brief   Firmware upgrade module
 *
 * We use two separate images, one is used to launch the application, the second
 * is used to store the data during the FW update. The bootloader decides if the
 * data in the second partition are valid and if so, the second partition
 * is copied to the first one and booted. Else the first partition is booted
 * directly
 */
#include <string.h>
#include <types.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include "utils/crc.h"
#include "hal/flash.h"
#include "hal/reloc.h"
#include "modules/fw.h"
#include "app.h"

/* These symbols are to be defined in a linker file */
extern const char *fw_img_addr;
extern const char *fw_img2_addr;
extern const char *fw_img_size;

/** Image Header size */
#define FW_HDR_SIZE           0x80U /* align at 7 bits for VTOR settings */
/** Size of the image area including fw header, aligned to page boundary */
#define FW_IMG_SIZE           ((uint32_t)&fw_img_size)
/** Max size of the image itself */
#define FW_IMG_DATA_SIZE      (FW_IMG_SIZE - FW_HDR_SIZE)
/** Address of the image header */
#define FW_IMG_HDR_ADDR(img)  ((uint32_t)(img == 0 ? &fw_img_addr : &fw_img2_addr))
/** Address of the image itself */
#define FW_IMG_DATA_ADDR(img) (FW_IMG_HDR_ADDR(img) + FW_HDR_SIZE)

/** Header describing stored firmware image */
typedef struct {
    uint32_t magic;
    uint32_t len;
    uint16_t crc;
} fw_hdr_t;

/** Firmware version information structure */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint32_t magic;
} __attribute__((packed)) fw_version_t;

typedef struct {
    uint16_t crc;
    uint32_t len;
    uint32_t last_erased;
    uint32_t last_written;
    bool running;
} fw_update_t;

/** Firmware version encoded in image binary */
__attribute__((section(".fw_version")))
const fw_version_t fw_version = { FW_MAJOR, FW_MINOR, FW_MAGIC };

/** Pointer to application to jump to */
typedef void (*app_t)(void);

static fw_update_t fwi_update;

/**
 * Load image header
 *
 * @param img   Image number
 * @param hdr   Address to store header to
 */
static void Fwi_GetImgHeader(uint8_t img, fw_hdr_t *hdr)
{
    uint8_t *src = (uint8_t *)FW_IMG_HDR_ADDR(img);
    memcpy((uint8_t *)hdr, src, sizeof(fw_hdr_t));
}

/**
 * Check if image has valid length and CRC is correct
 *
 * @param img       Image number
 * @return True if valid
 */
static bool Fwi_CheckImgValid(uint8_t img)
{
    fw_hdr_t hdr;
    uint16_t crc;

    Fwi_GetImgHeader(img, &hdr);

    if (hdr.magic != FW_MAGIC) {
        return false;
    }
    if (hdr.len > FW_IMG_DATA_SIZE) {
        return false;
    }

    crc = CRC16((uint8_t *)FW_IMG_DATA_ADDR(img), hdr.len);
    if (crc != hdr.crc) {
        return false;
    }

    return true;
}

/**
 * Copy the update image into runnable area
 */
static void Fwi_CopyImage(void)
{
    fw_hdr_t hdr;
    uint32_t end;
    uint32_t page_size;
    uint32_t addr = FW_IMG_HDR_ADDR(0);
    uint8_t *p = (uint8_t *)FW_IMG_HDR_ADDR(1);

    Fwi_GetImgHeader(1, &hdr);
    end = FW_IMG_DATA_ADDR(1) + hdr.len;

    page_size = Flashd_GetPageSize();
    Flashd_WriteEnable();
    while ((uint32_t)p < end) {
        Flashd_ErasePage(addr);
        Flashd_Write(addr, p, page_size);
        addr += page_size;
        p += page_size;
    }
    Flashd_WriteDisable();
}

void Fw_Run(void)
{
    fw_hdr_t hdr_run;
    fw_hdr_t hdr_img;
    bool img_valid;

    Fwi_GetImgHeader(0, &hdr_run);
    Fwi_GetImgHeader(1, &hdr_img);
    img_valid = Fwi_CheckImgValid(1);

    if ((hdr_run.crc != hdr_img.crc) && img_valid) {
        Fwi_CopyImage();
    } else if (!Fwi_CheckImgValid(0) && img_valid) {
        Fwi_CopyImage();
    }

    Reloc_RunFwBinary(FW_IMG_DATA_ADDR(0));
}

bool Fw_UpdateInit(uint16_t crc, uint32_t len)
{
    uint32_t addr = FW_IMG_HDR_ADDR(1);
    /* Image must start at page boundary */
    ASSERT(addr % Flashd_GetPageSize() == 0);

    if (len > FW_IMG_DATA_SIZE) {
        return false;
    }

    fwi_update.crc = crc;
    fwi_update.last_erased = addr + Flashd_GetPageSize();
    fwi_update.last_written = 0;
    fwi_update.len = len;
    fwi_update.running = true;

    Flashd_WriteEnable();
    /* Erase image header */
    Flashd_ErasePage(addr);
    return true;
}

bool Fw_Update(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    static uint8_t add_byte;
    uint32_t erase_end = FW_IMG_DATA_ADDR(1) + addr + len;
    uint32_t write_addr = FW_IMG_DATA_ADDR(1) + addr;

    if (!fwi_update.running) {
        return false;
    }
    if (addr != fwi_update.last_written || addr + len > fwi_update.len) {
        Fw_UpdateAbort();
        return false;
    }

    while (fwi_update.last_erased < erase_end) {
        Flashd_ErasePage(fwi_update.last_erased);
        fwi_update.last_erased += Flashd_GetPageSize();
    }

    fwi_update.last_written += len;
    /* Flash has to be written in 2 byte chunks on even addresses */
    if (len && write_addr & 0x1) {
        uint16_t val = (*buf) << 8 | add_byte;
        Flashd_Write(write_addr - 1, (uint8_t *)&val, 2);
        len--;
        buf++;
        write_addr++;
    }
    if (len & 0x1 && addr + len < fwi_update.len) {
        len--;
        add_byte = *(buf + len);
    }
    Flashd_Write(write_addr, buf, len);
    return true;
}

void Fw_UpdateAbort(void)
{
    if (fwi_update.running) {
        fwi_update.running = false;
        Flashd_WriteDisable();
    }
}

bool Fw_UpdateFinish(void)
{
    uint16_t crc;
    fw_hdr_t hdr;

    if (!fwi_update.running) {
        return false;
    }
    if (fwi_update.last_written != fwi_update.len) {
        Fw_UpdateAbort();
        return false;
    }

    crc = CRC16((uint8_t *)FW_IMG_DATA_ADDR(1), fwi_update.len);
    if (fwi_update.crc != crc) {
        Fw_UpdateAbort();
        return false;
    }

    fwi_update.running = false;
    hdr.crc = fwi_update.crc;
    hdr.len = fwi_update.len;
    hdr.magic = FW_MAGIC;

    Flashd_Write(FW_IMG_HDR_ADDR(1), (uint8_t *)&hdr, sizeof(hdr));
    Flashd_WriteDisable();
    return true;
}

bool Fw_UpdateIsRunning(void)
{
    return fwi_update.running;
}

uint8_t *Fw_GetCurrent(uint32_t *length, uint32_t *crc)
{
    fw_hdr_t hdr;
    Fwi_GetImgHeader(0, &hdr);

    if (length != NULL) {
        *length = hdr.len;
    }
    if (crc != NULL) {
        *crc = hdr.crc;
    }
    return (uint8_t *)FW_IMG_DATA_ADDR(0);
}
