# Another Firmware Library

AFW is a group of cooperating sources for speeding up a firmware development
and unifying access to MCU peripherals. It contains common sources I use in
most MCU projects

# Content
 * Lightweight HAL for STM32 (built on top of [libopencm3](libopencm3.org))
 * Drivers for various sensors, displays, etc
 * Simple logger
 * FW update mechanism
 * FAT16 virtual ramdisk
 * Various protocols (NMEA, LoRaWanMAC,...)
 * Tiny library for graphical displays
 * Naive implementation of AES128
 * Utilities to work with time, buttons debouncing, simplified math, ring buffer,...

# Files
Sources
 * *drivers* - drivers for sensors, displays,...
 * *external* - externally linked projects
 * *hal* - Custom STM32 HAL
 * *modules* - Reusable modules, e.g. logging, protocols,...
 * *modules/cgui* - Custom gui library for monochromatic displays
 * *utils* - Common utilities (time, crc, aes128,...)
 * *types.h* - Common types definition

Templates
 * *Makefile.tmpl* - Makefile template for projects using this library
 * *config_idems.ods* - Spreadsheet for *tools/config_items.py* generator

Tools
 * *mx2board.py* - tool to generate pinmux configuration (from [ChibiOS-Contrib](https://github.com/ChibiOS/ChibiOS-Contrib/))
 * *config_items.py* - Generator for configuration options header for *sources/modules/config.c*
 * *bin2uf2.py* - uf2 binary format generator
 * *fw.py* - Firmware images and image headers in *sources/modules/fw.c* compatible format
 * *templates* - templates for code generators
 * *cgui* - generator of fonts and image converter for cgui library

Tests
 * Unit tests for the library

# Development

Install necessary tooling
 * make
 * clang-format
 * pre-commit
 * docker

Instal precommit for this repository (automated code checking and tests execution):
 * `pre-commit install`

Tests can be launched by
 * `cd tests`
 * `make` - this builds docker container for ceedling if doesn't exist and launches tests

Code can be autoformatted to selected code standard by
 * `clang-format -i style=file path/to/file.c`
