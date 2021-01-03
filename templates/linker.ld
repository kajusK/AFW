/* Linker script for STM32F05xz8, 64k flash, 8k RAM. */

/* Define memory regions. */
MEMORY
{
    /* 64 kB decreased by 2 (1024 bytes long) pages for config storage */ 
    rom (rx) : ORIGIN = 0x08000000, LENGTH = 64K - 2K
    ram (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
}

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld

PROVIDE(_config_part1 = ORIGIN(rom) + LENGTH(rom));
PROVIDE(_config_part2 = ORIGIN(rom) + LENGTH(rom)+1K);
