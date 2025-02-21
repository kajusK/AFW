/**
 * @file    hal/flash.c
 * @brief   Internal flash memory driver
 */

#include <types.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/desig.h>
#include "hal/flash.h"

uint32_t Flashd_GetFlashSize(void)
{
    return desig_get_flash_size()*1024;
}

uint32_t Flashd_GetPageSize(void)
{
    /* Might not be valid for all MCUs, but should work for at least for F0 */
    if (desig_get_flash_size() >= 128) {
        return 2048;
    }
    return 1024;
}

void Flashd_WriteEnable(void)
{
    flash_unlock();
}

void Flashd_WriteDisable(void)
{
    flash_lock();
}

void Flashd_ErasePage(uint32_t addr)
{
    flash_erase_page(addr);
}

void Flashd_Write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    /* Writes must be 2 byte aligned */
    ASSERT_NOT(addr & 0x1)

    while (len >= 2) {
        flash_program_half_word(addr, (*(buf+1)) << 8 | *buf);
        addr += 2;
        buf += 2;
        len -= 2;
    }

    /*
     * len was not multiply of two,
     * warning -second byte can't be written afterwards, even if is 0xff
     */
    if (len != 0) {
        flash_program_half_word(addr, 0xff00 | *buf);
    }
}
