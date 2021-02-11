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

PROVIDE(_config_part1 = ORIGIN(config1));
PROVIDE(_config_part2 = ORIGIN(config2));
PROVIDE(fw_img_addr = ORIGIN(fw_img));
PROVIDE(fw_img2_addr = ORIGIN(fw_img2));
PROVIDE(fw_img_size = ORIGIN(fw_img));


EXTERN (vector_table)
ENTRY(reset_handler)

SECTIONS
{
	.text : {
		*(.vectors)

		/* Place fw header here - offset 0x100 */
		. = 256;
		KEEP(*(.fw_version))	

		. = ALIGN(4);
		*(.text*)
		. = ALIGN(4);
		*(.rodata*)	
		. = ALIGN(4);
	} >rom

	/* C++ Static constructors/destructors */ 
	.preinit_array : {
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP (*(.preinit_array))
		__preinit_array_end = .;
	} >rom
	.init_array : {
		. = ALIGN(4);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	} >rom
	.fini_array : {
		. = ALIGN(4);
		__fini_array_start = .;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		__fini_array_end = .;
	} >rom

	/* C++ stuff, appears when using newlib with (long long) printf support */
	.ARM.extab : {
		*(.ARM.extab*)
	} >rom
	.ARM.exidx : {
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} >rom

	. = ALIGN(4);
	_etext = .;

	/* ram, but not cleared on reset, eg boot/app comms */
	.noinit (NOLOAD) : {
		*(.noinit*)
	} >ram
	. = ALIGN(4);

	.data : {
		_data = .;
		*(.data*)
		*(.ramtext*)    /* "text" functions to run in ram */
		. = ALIGN(4);
		_edata = .;
	} >ram AT >rom
	_data_loadaddr = LOADADDR(.data);

	.bss : {
		*(.bss*)	
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >ram

	/*
	 * The .eh_frame section appears to be used for C++ exception handling.
	 * You may need to fix this if you're using C++.
	 */
	/DISCARD/ : { *(.eh_frame) }

	. = ALIGN(4);
	end = .;
}

PROVIDE(_stack = ORIGIN(ram) + LENGTH(ram));
