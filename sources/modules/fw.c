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
 * @file    modules/fw.c
 * @brief   Firmware upgrade module
 *
 * @addtogroup modules
 * @{
 */
#include <string.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/cortex.h>
#include "utils/assert.h"
#include "utils/crc.h"
#include "hal/flash.h"
#include "modules/fw.h"

/** Total amount of fw images */
#define FW_IMG_COUNT                2
/** Area reserved for bootloader */
#define FW_BL_RESERVED              0x1000
/** Area reserved for bootloader */
#define FW_HDR_SIZE                 FLASHD_PAGE_SIZE
/** Size of the image area including fw header */
#define FW_IMG_SIZE                 ((FLASHD_SIZE - FW_BL_RESERVED)/2)
/** Max size of the image itself */
#define FW_IMG_DATA_SIZE            (FW_IMG_SIZE - FW_HDR_SIZE)
/** Address of the image header */
#define FW_IMG_HDR_ADDR(img)        ((img) * FW_IMG_SIZE + FW_BL_RESERVED)
/** Address of the image itself */
#define FW_IMG_DATA_ADDR(img)       ((img) * FW_IMG_SIZE + FW_HDR_SIZE + FW_BL_RESERVED)

/** Image was not booted yet */
#define FW_FLAG_FIRST_BOOT 0x01
/** Image was not yet verified by itself */
#define FW_FLAG_VERIFIED 0x02
/** Image is the latest one flashed */
#define FW_FLAG_LATEST 0x04

/** Header describing stored firmware image */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint16_t crc;
    uint32_t len;
    uint8_t flags;
} fw_hdr_t;

/** Info about currently running fw update */
typedef struct {
    fw_hdr_t hdr;
    uint32_t written;
    uint8_t img;
    bool running;
} fw_update_t;

static fw_update_t fwi_update;

/**
 * Set stack pointer and jump to fw image on given address
 *
 * @param address   Address of the image in the flash memory
 */
static void Fwi_JumpToApp(uint32_t addr)
{
    (void) addr;
    /* Set stack pointer to initial stack address */
    asm("mov sp, r0");
    /* Can't access pc directly in thumb mode */
    asm("ldr r7, [r0, #4]");
    /* Jump to app */
    asm("mov pc, r7");
}

/**
 * Load image header
 *
 * @param img   Image number
 * @param hdr   Address to store header to
 */
static void Fw_GetImgHeader(uint8_t img, fw_hdr_t *hdr)
{
    uint8_t *src = (uint8_t *) FW_IMG_HDR_ADDR(img);
    memcpy((uint8_t *)hdr, src, sizeof(fw_hdr_t));
}

/**
 * Write image header (only bits in 1 are updated)
 *
 * @param img   Image number
 * @param hdr   Image header
 */
static void Fw_SetImgHeader(uint8_t img, const fw_hdr_t *hdr)
{
    Flashd_Write(FW_IMG_HDR_ADDR(img), (uint8_t *) hdr, sizeof(fw_hdr_t));
}

/**
 * Clear latest flags of all images except img
 *
 * @param img   Image that should have latest flag unchanged
 */
static void Fw_SetLatest(uint8_t img)
{
    fw_hdr_t hdr;

    for (uint8_t i = 0; i < FW_IMG_COUNT; i++) {
        if (i == img) {
            continue;
        }
        Fw_GetImgHeader(img, &hdr);
        if (hdr.flags & FW_FLAG_LATEST) {
            hdr.flags &= ~FW_FLAG_LATEST;
            Fw_SetImgHeader(i, &hdr);
        }
    }
}

/**
 * Check if image has valid length and CRC is correct
 *
 * @param img       Image number
 * @return True if valid
 */
static bool Fw_CheckImgValid(uint8_t img)
{
    fw_hdr_t hdr;
    uint16_t crc;

    Fw_GetImgHeader(img, &hdr);

    if (hdr.len > FW_IMG_DATA_SIZE) {
        return false;
    }

    crc = CRC16((uint8_t *) FW_IMG_DATA_ADDR(fwi_update.img), hdr.len);
    if (crc != hdr.crc) {
        return false;
    }

    return true;
}

/**
 * Get ID of the currently running image
 *
 * @return Image number or 0xff if failed (bootloader?)
 */
static uint8_t Fw_GetRunningImg(void)
{
    /* Use address of this function to determine partition we are running from */
    uint32_t addr = (uint32_t) Fw_GetRunningImg;

    if (addr >= FW_IMG_DATA_ADDR(0) && addr < FW_IMG_HDR_ADDR(1)) {
        return 0;
    }
    if (addr >= FW_IMG_DATA_ADDR(1)) {
        return 1;
    }
    return 0xff;
}

/**
 * Select fw image to be rewritten by fw update
 *
 * @return Image number or 0xff if failed
 */
static uint8_t Fw_SelectUpdateImg(void)
{
    uint8_t running = Fw_GetRunningImg();
    if (running == 0xff) {
        //TODO called from bootloader???
        return 0;
    }

    return (running + 1) % FW_IMG_COUNT;
}
/**
 * Select fw image to be booted
 *
 * @return Image number or 0xff if failed
 */
static uint8_t Fw_SelectBootImg(void)
{
    fw_hdr_t hdr[FW_IMG_COUNT];
    uint8_t i;

    for (i = 0; i < FW_IMG_COUNT; i++) {
        Fw_GetImgHeader(i, &hdr[i]);
    }

    /* First check for images that were not booted yet */
    for (i = 0; i < FW_IMG_COUNT; i++) {
        /* not booted yet, this one should be launched */
        if (hdr[i].flags & FW_FLAG_FIRST_BOOT) {
            /* verify image has valid crc */
            if (Fw_CheckImgValid(i)) {
                return i;
            }
        }
    }

    /* Check for image that is marked as latest and is verified */
    for (i = 0; i < FW_IMG_COUNT; i++) {
        if (hdr[i].flags & FW_FLAG_LATEST && hdr[i].flags & FW_FLAG_VERIFIED) {
            if (Fw_CheckImgValid(i)) {
                return i;
            }
        }
    }

    /* Try any verified image */
    for (i = 0; i < FW_IMG_COUNT; i++) {
        if (hdr[i].flags & FW_FLAG_VERIFIED) {
            if (Fw_CheckImgValid(i)) {
                return i;
            }
        }
    }

    /* last chance, try to boot image that has valid crc */
    for (i = 0; i < FW_IMG_COUNT; i++) {
        if (Fw_CheckImgValid(i)) {
            return i;
        }
    }

    return 0xff;
}

void Fw_Run(void)
{
    uint8_t img = Fw_SelectBootImg();
    fw_hdr_t hdr;
    uint32_t addr;

    /* No valid image found */
    if (img == 0xff) {
        return;
    }

    Fw_GetImgHeader(img, &hdr);
    if (hdr.flags & FW_FLAG_FIRST_BOOT) {
        hdr.flags &= ~FW_FLAG_FIRST_BOOT;
        Fw_SetImgHeader(img, &hdr);
    }

    addr = FW_IMG_DATA_ADDR(img);

    cm_disable_interrupts();
    /* Set vector table address to fw image */
    SCB_VTOR = addr << SCB_VTOR_TBLOFF_LSB;
    /* Set initial stack pointer */
    Fwi_JumpToApp(addr);
}

void Fw_Reboot(void)
{
    scb_reset_system();
    while(1);
}

bool Fw_UpdateInit(uint8_t major, uint8_t minor, uint16_t crc, uint32_t len)
{
    uint32_t addr = 0;
    uint8_t img;

    if (len > FW_IMG_DATA_SIZE) {
        return false;
    }
    img = Fw_SelectUpdateImg();

    fwi_update.hdr.major = major;
    fwi_update.hdr.minor = minor;
    fwi_update.hdr.crc = crc;
    fwi_update.hdr.len = len;
    fwi_update.hdr.flags = 0xff;
    fwi_update.written = 0;
    fwi_update.img = img;
    fwi_update.running = true;

    /* Erase image */
    Flashd_ErasePage(FW_IMG_HDR_ADDR(img));
    while (addr < len) {
        Flashd_ErasePage(FW_IMG_HDR_ADDR(img) + addr);
        addr += FLASHD_PAGE_SIZE;
    }
    return true;
}

void Fw_Update(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    if (addr + len > FW_IMG_DATA_SIZE) {
        return;
    }
    if (!fwi_update.running) {
        return;
    }

    Flashd_Write(addr + FW_IMG_DATA_ADDR(fwi_update.img), buf, len);
    fwi_update.written += len;
}

bool Fw_UpdateFinish(void)
{
    uint8_t *addr;
    uint16_t crc;

    if (!fwi_update.running) {
        return false;
    }
    fwi_update.running = false;

    if (fwi_update.written != fwi_update.hdr.len) {
        return false;
    }

    addr = (uint8_t *) FW_IMG_DATA_ADDR(fwi_update.img);
    crc = CRC16(addr, fwi_update.hdr.len);

    if (fwi_update.hdr.crc != crc) {
        return false;
    }

    Fw_SetImgHeader(fwi_update.img, &fwi_update.hdr);
    Fw_SetLatest(fwi_update.img);
    return true;
}

/** @} */
