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
 * We use two separate images, one is used to launch the application, the second
 * is used to store the data during the FW update. The bootloader decides if the
 * data in the second partition are valid and if so, the second partition
 * is copied to the first one and booted. Else the first partition is booted
 * directly
 *
 * @addtogroup modules
 * @{
 */
#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include "utils/assert.h"
#include "utils/crc.h"
#include "hal/flash.h"
#include "modules/fw.h"

/* These symbols are to be defined in a linker file */
extern const char fw_img_addr;
extern const char fw_img2_addr;
extern const char fw_img_size;

/** Start of RAM memory */
#define FW_RAM_START                0x20000000U
/** Image Header size */
#define FW_HDR_SIZE                 0x80U /* align at 7 bits for VTOR settings */
/** Size of the image area including fw header, aligned to page boundary */
#define FW_IMG_SIZE                 ((uint32_t)&fw_img_size)
/** Max size of the image itself */
#define FW_IMG_DATA_SIZE            (FW_IMG_SIZE - FW_HDR_SIZE)
/** Address of the image header */
#define FW_IMG_HDR_ADDR(img)        ((uint32_t)(img == 0 ? &fw_img_addr : &fw_img2_addr))
/** Address of the image itself */
#define FW_IMG_DATA_ADDR(img)       (FW_IMG_HDR_ADDR(img) + FW_HDR_SIZE)
/** Magic number for the image header signature */
#define FW_MAGIC 0xDEADBEEF

/** Header describing stored firmware image */
typedef struct {
    uint32_t magic;
    uint32_t len;
    uint16_t crc;
} fw_hdr_t;

typedef struct {
    uint16_t crc;
    uint32_t len;
    uint32_t last_erased;
    uint32_t last_written;
    bool running;
} fw_update_t;

/** Pointer to application to jump to */
typedef void (*app_t)(void);

static fw_update_t fwi_update;

/**
 * Replace current vector table with fw image one
 *
 * @param addr      Address of the firmware image vector table (address 0)
 */
static void Fwi_RelocateVectors(uint32_t addr)
{
    uint32_t cpuid = SCB_CPUID;
    if (((cpuid & SCB_CPUID_IMPLEMENTER) >> SCB_CPUID_IMPLEMENTER_LSB) == 0x41 &&
            ((cpuid & SCB_CPUID_VARIANT) >> SCB_CPUID_VARIANT_LSB) == 0x00 &&
            ((cpuid & SCB_CPUID_CONSTANT) >> SCB_CPUID_CONSTANT_LSB) == 0xc &&
            ((cpuid & SCB_CPUID_PARTNO) >> SCB_CPUID_PARTNO_LSB) == 0xc20) {
        /*
         * The Cortex-M0 doesn't have VTOR register, move vector table
         * to beggining of RAM and remap RAM to address 0x0
         * In this case, first XY bytes of RAM must be reserved in image linker
         */
        /* Cortex-M0 has 48 interrupt handlers, 4 bytes wide */
        memcpy((void *)FW_RAM_START, (void *)addr, 0xC0);

        /* Clock for SYSCFG must be enabled for remmaping to work */
        rcc_periph_clock_enable(RCC_SYSCFG_COMP);

        /* Remap RAM to 0x0 */
        SYSCFG_CFGR1 &= SYSCFG_CFGR1_MEM_MODE;
        SYSCFG_CFGR1 |= SYSCFG_CFGR1_MEM_MODE_SRAM;
    } else {
        /* If running other core than M0, the VTOR can be use to point to a new
         * vector table. The address must be 7 bit aligned though */
        ASSERT((addr & 0x7f) == 0);
        SCB_VTOR = addr << SCB_VTOR_TBLOFF_LSB;
    }
}

/**
 * Set stack pointer and jump to fw image on given address
 *
 * @param address   Address of the image in the flash memory
 */
static void Fwi_JumpToApp(uint32_t addr)
{
    uint32_t app = *((uint32_t *)(addr + 4));
    uint32_t sp = *((uint32_t *)addr);

    /* Disable all interrupts */
    for (uint8_t i = 0; i < NVIC_IRQ_COUNT; i++) {
        nvic_disable_irq(i);
    }
    /*
     * Memory/instruction barrier to make sure pending interrupts
     * were not triggered
     */
    asm volatile("dsb");
    asm volatile("isb");

    Fwi_RelocateVectors(addr);
    asm volatile("mov sp, %0" : : "r" (sp));
    asm volatile("bx %0" : : "r" (app));
}

/**
 * Load image header
 *
 * @param img   Image number
 * @param hdr   Address to store header to
 */
static void Fwi_GetImgHeader(uint8_t img, fw_hdr_t *hdr)
{
    uint8_t *src = (uint8_t *) FW_IMG_HDR_ADDR(img);
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

    crc = CRC16((uint8_t *) FW_IMG_DATA_ADDR(img), hdr.len);
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
    uint8_t *p = (uint8_t *) FW_IMG_HDR_ADDR(1);

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

    Fwi_JumpToApp(FW_IMG_DATA_ADDR(0));
}

void Fw_Reboot(void)
{
    scb_reset_system();
    while(1) {
        ;
    }
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
        Flashd_Write(write_addr-1, (uint8_t *)&val, 2);
        len--;
        buf++;
        write_addr++;
    }
    if (len & 0x1 && addr + len < fwi_update.len) {
        len--;
        add_byte = *(buf+len);
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

    crc = CRC16((uint8_t *) FW_IMG_DATA_ADDR(1), fwi_update.len);
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
    return (uint8_t *) FW_IMG_DATA_ADDR(0);
}

/** @} */
