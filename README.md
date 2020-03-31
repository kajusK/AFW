Another Firmware Library
========================

AFW is a group of cooperating sources for speeding up a firmware development
and unifying access to MCU peripherals. It contains common sources I use in
most MCU projects

Content
-------
 * Lightweight HAL for STM32 (built on top of [libopencm3](libopencm3.org))
 * Drivers for various sensors, displays, etc
 * Simple logger
 * Various protocols (NMEA, LoRaWanMAC,...)
 * Tiny library for graphical displays
 * Naive implementation of AES128
 * Utilities to work with time, buttons debouncing, simplified math, ring buffer,...

Files:
------
 * *Makefile.tmpl* - Makefile template for projects using this library
 * *afwconfig.h.tmpl* - Template configuration file

Sources
 * *drivers* - drivers for sensors, displays,...
 * *external* - subrepos for unit testing and libopencm3
 * *hal* - Custom STM32 HAL
 * *modules* - Reusable modules, e.g. logging, protocols,...
 * *modules/cgui* - Custom gui library for monochromatic displays
 * *utils* - Common utilities (time, crc, aes128,...)
 * *types.h* - Common types definition

Tools
 * *mx2board.py* - tool to generate pinmux configuration (from [ChibiOS-Contrib](https://github.com/ChibiOS/ChibiOS-Contrib/))
 * *templates* - templates for code generators
 * *cgui* - generator of fonts and image converter for cgui library
 
Tests
 * Unit tests for the library 
