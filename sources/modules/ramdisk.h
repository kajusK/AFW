/**
 * @file    modules/ramdisk.h
 * @brief   RAMdisk emulation with on the fly file creation, uses FAT16
 */

#ifndef __MODULES_RAMDISK_H
#define __MODULES_RAMDISK_H

#include <types.h>
#include <time.h>

/* Amount of files that can be stored in ramdisk root directory */
#define RAMDISK_MAX_FILES 4

/**
 * Type for function to read data from virtual file
 * @param offset    Offset in bytes to read data from
 * @param buf       Buffer to read data to
 * @param len       Amount of bytes to read
 */
typedef void (* ramdisk_read_t)(uint32_t offset, uint8_t *buf, size_t len);

/**
 * Type for function to write a data to ramdisk virtual file
 *
 * @param name      File name
 * @param extension File name extension
 * @param file_size File size
 * @param buf       Data to be written (usually 512 bytes)
 * @param size      Size of the buf
 * @param offset    Offset from file start
 */
typedef void (* ramdisk_write_file_cb_t)(const uint8_t *buf, size_t size,
        uint32_t offset);

/**
 * Read data from ramdisk
 *
 * @param lba   Logical Block Address, address of the sector (512 Bytes long)
 * @param buf   Buffer 512 bytes long for data
 *
 * @return 0
 */
int Ramdisk_Read(uint32_t lba, uint8_t *buf);

/**
 * Write data to ramdisk
 *
 * @param lba   Logical Block Address, address of the sector (512 Bytes long)
 * @param buf   Data to be written
 *
 * @return 0
 */
int Ramdisk_Write(uint32_t lba, const uint8_t *buf);

/**
 * Create a new file in ramdisk
 *
 * @param filename      Up to 8 characters of file name
 * @param extension     3 characters extension
 * @param time          Created timestamp
 * @param size          File size in bytes
 * @param read          Function to read data from file
 * @return  -1 or non negative file handle
 */
int Ramdisk_AddFile(const char *filename, const char *extension,
        time_t time, size_t size, ramdisk_read_t read);

/**
 * Add a simple text file with static content
 *
 * @param filename      Up to 8 characters of file name
 * @param extension     3 characters extension
 * @param time          Created timestamp
 * @param text          Null terminated string as file content
 * @return  -1 or non negative file handle
 */
int Ramdisk_AddTextFile(const char *filename, const char *extension,
        time_t time, const char *text);

/**
 * Rename the existing file
 *
 * Warning - the change might not be propagated correctly when the ramdisk
 * is currently used from outside. Reconnecting is suggested (e.g. unplug
 * and plug the USB when used in USB MSC,...)
 *
 * @param handle    File handle obtained during file creation
 * @param filename  Up to 8 characters extension
 * @param extension Up to 3 character extension
 * @return Successfulness of the operation
 */
bool Ramdisk_RenameFile(int handle, const char *filename,
        const char *extension);

/**
 * Clear ramdisk content
 */
void Ramdisk_Clear(void);

/**
 * Get size of the ramdisk
 *
 * @return Amount of sectors in volume (512 bytes long)
 */
uint32_t Ramdisk_GetSectors(void);

/**
 * Register callback for writing to file
 *
 * @param cb    Callback
 */
void Ramdisk_RegisterWriteCb(ramdisk_write_file_cb_t cb);

/**
 * Initialize ramdisk of given size
 *
 * @param size  Ramdisk size in bytes or zero for default (may be increased for minimum fat16 size)
 * @param name  Volume name (up to 11 characters are used)
 */
void Ramdisk_Init(size_t size, const char *name);

#endif
