/**
 * @file    hal/flash.h
 * @brief   Internal flash memory driver
 */

#ifndef __HAL_FLASH_H_
#define __HAL_FLASH_H_

#include <types.h>
#include <libopencm3/stm32/memorymap.h>

/** Flash address memory map */
#define FLASHD_START              FLASH_BASE

/**
 * Get size of the flash memory in the MCU in bytes
 */
uint32_t Flashd_GetFlashSize(void);

/**
 * Get size of the memory page in the MCU in bytes
 */
uint32_t Flashd_GetPageSize(void);

/**
 * Enable writing to the flash memory
 */
void Flashd_WriteEnable(void);

/**
 * Disable writing to the flash memory
 */
void Flashd_WriteDisable(void);

/**
 * Erase given flash page
 */
void Flashd_ErasePage(uint32_t addr);

/**
 * Write data to the internal flash
 *
 * Unfortunately the stm32 limits writes to even addresses in 2 byte chunks,
 * once the data on the address is not 0xffff, it cannot be updated again.
 *
 * @param addr  Start address (must be an even address)
 * @param buf   Data buffer
 * @param len   Amount of bytes to be written
 */
void Flashd_Write(uint32_t addr, const uint8_t *buf, uint32_t len);

#endif
