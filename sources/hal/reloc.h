/**
 * @file    hal/power.h
 * @brief   Support for bootloader - vector table relocation and running an user app
 */

#ifndef __HAL_RELOC_H
#define __HAL_RELOC_H

#include <types.h>

/**
 * Replace current vector table with new one
 *
 * @note on MCUs without VTOR support, RAM is used, reserve beginning of RAM for vector table
 *
 * @param addr      Address of the vector table to be used, 7 bit aligned on VTOR compatible
 * architectures
 */
void Reloc_VectorTable(uint32_t addr);

/**
 * Set stack pointer, relocate vector table and jump to fw on given address
 *
 * @param address   Address of the firmware in the flash memory
 */
void Reloc_RunFwBinary(uint32_t addr);

#endif
