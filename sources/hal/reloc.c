/**
 * @file    hal/reloc.c
 * @brief   Support for bootloader - vector table relocation and running an user app
 */

#include <types.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/syscfg.h>
#include <libopencm3/stm32/memorymap.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/cortex.h>
#include "reloc.h"

/** Start of RAM memory */
#define RAM_BASE 0x20000000U

void Reloc_VectorTable(uint32_t addr)
{
    uint32_t cpuid = SCB_CPUID;
    if (((cpuid & SCB_CPUID_IMPLEMENTER) >> SCB_CPUID_IMPLEMENTER_LSB) == 0x41 &&
        ((cpuid & SCB_CPUID_VARIANT) >> SCB_CPUID_VARIANT_LSB) == 0x00 &&
        ((cpuid & SCB_CPUID_CONSTANT) >> SCB_CPUID_CONSTANT_LSB) == 0xc &&
        ((cpuid & SCB_CPUID_PARTNO) >> SCB_CPUID_PARTNO_LSB) == 0xc20)
    {
        /*
         * The Cortex-M0 doesn't have VTOR register, move vector table
         * to beggining of RAM and remap RAM to address 0x0
         * In this case, first XY bytes of RAM must be reserved in image linker
         */
        /* Cortex-M0 has 48 interrupt handlers, 4 bytes wide */
        uint8_t bytes = 0xc0;
        uint8_t *ram = (uint8_t *)RAM_BASE;
        uint8_t *table = (uint8_t *)addr;
        while (bytes--) {
            *ram++ = *table++;
        }

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

void Reloc_RunFwBinary(uint32_t addr)
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

    Reloc_VectorTable(addr);
    asm volatile("mov sp, %0" : : "r"(sp));
    asm volatile("bx %0" : : "r"(app));
}
