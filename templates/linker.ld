/* Linker script for STM32F05xz8, 64k flash, 8k RAM. */

/* Define memory regions. */
MEMORY
{
    /* Real flash area */
    flash(rx) : ORIGIN = 0x08000000, LENGTH = 64K
	/* 0xc0 bytes reserved for relocated vector table (by bootloader) */
	ram (rwx) : ORIGIN = 0x200000c0, LENGTH = 8K - 0xc0

    /* Flash partitions */

    /* Bootloader area */
    bl (rx) : ORIGIN = ORIGIN(flash), LENGTH = 2K
    /* Firmware image area */
    fw_img (rx) : ORIGIN = ORIGIN(bl) + LENGTH(bl), LENGTH = (LENGTH(flash) - LENGTH(bl) - 2K)/2
    /* Firmware update image area */
    fw_img2 (rx) : ORIGIN = ORIGIN(fw_img) + LENGTH(fw_img), LENGTH = LENGTH(fw_img)
    /* Config partition */
	config1(rx) : ORIGIN = ORIGIN(fw_img2) + LENGTH(fw_img2), LENGTH = 1K
	config2(rx) : ORIGIN = ORIGIN(config1) + LENGTH(config1), LENGTH = LENGTH(config1)

    /* 
     * Actual firmware area for linker script
     * 0x80 is a fw header, it has to be 7 bit aligned for vector relocation
     */
	rom (rx) : ORIGIN = ORIGIN(fw_img) + 0x80, LENGTH = LENGTH(fw_img) - 0x80
}

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld

PROVIDE(_config_part1 = ORIGIN(config1));
PROVIDE(_config_part2 = ORIGIN(config2));
PROVIDE(fw_img_addr = ORIGIN(fw_img));
PROVIDE(fw_img2_addr = ORIGIN(fw_img2));
PROVIDE(fw_img_size = ORIGIN(fw_img));
