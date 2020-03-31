/*
 * Copyright (C) 2019 Jakub Kaderka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file    drivers/ramdisk.h
 * @brief   RAMdisk emulation with on the fly file creation, uses FAT16
 *
 * FAT16 description http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html
 * http://www.tavi.co.uk/phobos/fat.html
 *
 * @addtogroup drivers
 * @{
 */

#include <string.h>
#include "utils/assert.h"
#include "utils/math.h"
#include "ramdisk.h"

/** 1, 2, 4, 8, 16, 32, 128 */
#define SECTORS_PER_CLUSTER 8
/** Files/subdirs in root directory */
#define ROOT_ENTRIES 512U

/* Do not change below defines */
/* bytes in sector */
#define SECTOR_SIZE 512U
#define CLUSTER_SIZE    (SECTOR_SIZE*SECTORS_PER_CLUSTER)
/* Size of single entry in directory */
#define DIR_ENTRY_SIZE 32

/* FAT16 has to have at least this amount of clusters to be recognized as fat16 */
#define FAT16_MIN_CLUSTERS 4095

/* First sector of the root directory */
#define FAT1_START_SECTOR   1U /* first sector is boot sector, followed by fat1 */
#define FAT2_START_SECTOR   (FAT1_START_SECTOR + ramdiski_info.fat_sectors)
#define ROOT_START_SECTOR   (FAT2_START_SECTOR + ramdiski_info.fat_sectors)
#define DATA_START_SECTOR   (ROOT_START_SECTOR + ceil_div(ROOT_ENTRIES*DIR_ENTRY_SIZE, SECTOR_SIZE))

#define FAT_2BYTES(x) ((x) & 0xFF), (((x) >> 8) & 0xFF)
#define FAT_4BYTES(x) ((x) & 0xFF), (((x) >> 8) & 0xFF),\
         (((x) >> 16) & 0xFF), (((x) >> 24) & 0xFF)

/* One entry reserved for volume label record */
#if RAMDISK_MAX_FILES > (ROOT_ENTRIES - 1)
    #error Too many files for ramdisk defined
#endif

/** Type for virtual files */
typedef struct {
    char name[8];       /** File name (padded with spaces) */
    char extension[3];  /** File extension */
    uint8_t time[2];    /** Time created in fat format */
    uint8_t date[2];    /** Date created in fat format */
    uint8_t attr;       /** File attributes */
    uint32_t size;      /** File size in bytes */
    uint16_t cluster;   /** First cluster of the file */
    ramdisk_read_t read;/** Function called upon read requests or NULL */
    const char *content;/** Text content of the file when read is NULL */
} ramdisk_file_t;

typedef struct {
    uint32_t sectors_count; /* size of volume in sectors */
    uint16_t fat_sectors;   /* size of fat table in sectors */
    char name[11];          /* volume label */
} ramdisk_info_t;

/** Files shown in ramdisk, if name starts with 0, file is ignored */
static ramdisk_file_t ramdiski_files[RAMDISK_MAX_FILES];

/** Parameters of the ramdisk that are calculated during runtime */
static ramdisk_info_t ramdiski_info;

/** FAT16 Boot sector header */
static const uint8_t fat16i_boot_sector[] = {
    0xeb, 0x3c, 0x90, /* bootstrap program */
    'm', 'k', 'd', 'o', 's', 'f', 's', 0x00, /* OEM ID */
    /* Bios parameters block */
    FAT_2BYTES(512), /* sector size */
    SECTORS_PER_CLUSTER, /* sectors per cluster */
    FAT_2BYTES(1), /* reserved sectors, (1 = boot sector only) */
    2, /* Number of FAT copies, usually 2 to prevent data loss */
    FAT_2BYTES(ROOT_ENTRIES), /* Number of root entries, usually 512 */
    FAT_2BYTES(0), /* Small number of sectors, for <32Mb partitions, or 0 */
    0xf8, /* Media descriptor, non-formated disk */
    FAT_2BYTES(1), /* Size of FAT table in sectors, overridden in code */
    FAT_2BYTES(63), /* Sectors per track, for physical disk geometry */
    FAT_2BYTES(255), /* Number of heads */
    FAT_4BYTES(0), /* Hidden sectors */
    FAT_4BYTES(0), /* Large number of sectors, partitions >32Mb, overridden in code */
    /* Extended bios parameters block */
    0x00, /* Drive number */
    0x00, /* Reserved */
    0x29, /* Extended boot signature */
    FAT_4BYTES(0xdeadbeef), /* Volume serial number */
    'r', 'a', 'm', 'd', 'i', 's', 'k', ' ', ' ', ' ', ' ', /* Volume label, overridden in code */
    'F', 'A', 'T', '1', '6', ' ', ' ', ' ' /* Filesystem type */
};

/**
 * Set two bytes value in little endian
 *
 * @param num       Number to be stored
 * @param buf       Buffer to store result to
 */
static void Ramdiski_To2Bytes(uint16_t num, uint8_t *buf)
{
    buf[0] = num & 0xff;
    buf[1] = (num >> 8) & 0xff;
}

/**
 * Set four bytes value in little endian
 *
 * @param num       Number to be stored
 * @param buf       Buffer to store result to
 */
static void Ramdiski_To4Bytes(uint32_t num, uint8_t *buf)
{
    buf[0] = num & 0xff;
    buf[1] = (num >> 8) & 0xff;
    buf[2] = (num >> 16) & 0xff;
    buf[3] = (num >> 24) & 0xff;
}

/**
 * Helper for generating text file, return one sector worth of file data
 *
 * @param id        File id (from internal structure)
 * @param offset    Offset in bytes to the file
 * @param buf       Buffer to store one sector to
 */
static void Ramdiski_ReadTextFile(int id, uint32_t offset, uint8_t *buf)
{
    ASSERT_NOT(ramdiski_files[id].content == NULL);

    if (offset >= ramdiski_files[id].size) {
        return;
    }

    strncpy((char *)buf, &ramdiski_files[id].content[offset], SECTOR_SIZE);
}

/**
 * Create a new file in ramdisk
 *
 * @param filename      Up to 8 characters of file name
 * @param extension     3 characters extension
 * @param time          Created timestamp
 * @param size          File size in bytes
 * @param read          Function to read data from file or NULL
 * @param content       String content of the file if read is NULL
 *
 * @return  True if succeeded (enough virtual space in ramdisk)
 */
static bool Ramdiski_AddFile(const char *filename, const char *extension,
        time_t time, size_t size, ramdisk_read_t read, const char *content)
{
    int id;
    struct tm *s_tm;
    uint32_t cluster = 2;

    ASSERT_NOT(filename == NULL || extension == NULL);

    /* Find first unused file entry */
    for (id = 0; id < RAMDISK_MAX_FILES; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            break;
        }
        cluster = (uint32_t)ramdiski_files[id].cluster +
                ramdiski_files[id].size/CLUSTER_SIZE + 1;
    }
    if (id >= RAMDISK_MAX_FILES) {
        return false;
    }

    /* check if there's enough clusters for the file */
    if (cluster + size/CLUSTER_SIZE >= 0xffef) {
        return false;
    }

    /* Set filename, pad with spaces */
    strncpy(ramdiski_files[id].name, filename, 8);
    for (size_t i = 7; i >= strlen(filename); i--) {
        ramdiski_files[id].name[i] = ' ';
    }

    /* Set file extension, pad with spaces */
    strncpy(ramdiski_files[id].extension, extension, 3);
    for (size_t i = 2; i >= strlen(extension); i--) {
        ramdiski_files[id].extension[i] = ' ';
    }

    /* set creation time and date in fat format */
    s_tm = localtime(&time);
    ramdiski_files[id].time[0] = s_tm->tm_sec / 2;
    ramdiski_files[id].time[0] |= (s_tm->tm_min & 0x07) << 5;
    ramdiski_files[id].time[1] = (s_tm->tm_min >> 3) & 0x07;
    ramdiski_files[id].time[1] |= s_tm->tm_hour << 3;
    ramdiski_files[id].date[0] = s_tm->tm_mday;
    ramdiski_files[id].date[0] |= ((s_tm->tm_mon + 1) & 0x07) << 5;
    ramdiski_files[id].date[1] = ((s_tm->tm_mon + 1) >> 3) & 0x01;
    ramdiski_files[id].date[1] |= (s_tm->tm_year - 80) << 1;

    ramdiski_files[id].cluster = cluster;
    ramdiski_files[id].size = size;
    ramdiski_files[id].attr = 0x01; /* read only */
    ramdiski_files[id].read = read;
    if (read == NULL) {
        ramdiski_files[id].content = content;
    }
    return true;
}

/**
 * Generate root directory sector content
 *
 * Each file has 32 bytes entry in root directory
 *
 * @param buf   Destination for generated data (512 bytes)
 * @param block Block offset relative to the first block of root entries (0-...)
 */
static void Ramdiski_GetRootDirectory(uint8_t *buf, uint32_t block)
{
    int id;
    uint16_t offset = 0;
    uint16_t start;

    memset(buf, 0x00, SECTOR_SIZE);
    /* First entry in the root directory is the volume label */
    if (block == 0) {
        memcpy(&buf[offset], ramdiski_info.name, 11);
        buf[offset+11] = 0x08; /* volume label attribute */
        offset += 32;
        start = 0;
    } else {
        /* first entry taken by the volume label entry */
        start = (SECTOR_SIZE/DIR_ENTRY_SIZE)*block - 1;
    }

    /* Each record is 32 bytes wide, so it aligns to 512B sectors nicely */
    for (id = start; id < RAMDISK_MAX_FILES && offset < SECTOR_SIZE; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            return;
        }
        memcpy(&buf[offset], ramdiski_files[id].name, 8);
        memcpy(&buf[offset+8], ramdiski_files[id].extension, 3);
        buf[offset+11] = ramdiski_files[id].attr;
        memcpy(&buf[offset+22], ramdiski_files[id].time, 2);
        memcpy(&buf[offset+24], ramdiski_files[id].date, 2);
        Ramdiski_To2Bytes(ramdiski_files[id].cluster, &buf[offset+26]);
        Ramdiski_To4Bytes(ramdiski_files[id].size, &buf[offset+28]);
        offset += 32;
    }
}

/**
 * Writing to root directory handler
 *
 * Only file creation is allowed, upon file creation, user callback is triggered
 * and file record is added to list of files
 *
 * @param buf   Data to be written (512 bytes)
 * @param block Block offset relative to the first block of root entries (0-...)
 */
static void Ramdiski_WriteRootDirectory(uint8_t *buf, uint32_t block)
{
    int id = 0;
    uint16_t offset = 0;
    uint16_t records = 1;

    for (id = 0; id < RAMDISK_MAX_FILES; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            break;
        }
    }
    /* first record is volume name */
    id++;
    /* Each record is 32 bytes long */
    offset = id*32;

    /* ignore writes to existing files records */
    if ((block + 1)*SECTOR_SIZE <= offset) {
        return;
    }

    /* offset should be multiply of 32 */
    offset -= block * SECTOR_SIZE;
    /* File name must not start with 0 = empty record */
    if (buf[offset] == 0x00) {
        return;
    }

    //TODO call file created function
}

/**
 * Generate FAT table for existing files
 *
 * https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#FAT
 *
 * @param buf   Destination for generated data (512 bytes)
 * @param block Block offset relative to the first block of FAT table (0-...)
 */
static void Ramdiski_GetFAT16(uint8_t *buf, uint32_t block)
{
    uint32_t i;
    uint16_t id;
    uint16_t offset; /* Offset to buf */
    uint16_t cluster; /* address of current cluster */

    memset(buf, 0x00, SECTOR_SIZE);
    if (block == 0) {
        /* Required initial header */
        buf[0] = 0xf8;
        buf[1] = 0xff;
        buf[2] = 0xff;
        buf[3] = 0xff;
        offset = 4;
        cluster = 2;
    } else {
        offset = 0;
        cluster = block*SECTOR_SIZE/2;
    }

    for (id = 0; id < RAMDISK_MAX_FILES && offset < SECTOR_SIZE; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            return;
        }
        /* Skip files which were stored to FAT in previous blocks */
        if (ramdiski_files[id].cluster + ramdiski_files[id].size/CLUSTER_SIZE < cluster) {
            continue;
        }

        if (ramdiski_files[id].size != CLUSTER_SIZE) {
            for (i = cluster - ramdiski_files[id].cluster;
                    i < ramdiski_files[id].size/CLUSTER_SIZE
                    && offset < SECTOR_SIZE; i++) {
                Ramdiski_To2Bytes(cluster + 1, &buf[offset]);
                offset += 2;
                cluster += 1;
            }
        }
        /* Last cluster of the file */
        if (offset < SECTOR_SIZE) {
            buf[offset] = 0xff;
            buf[offset+1] = 0xff;
            offset += 2;
            cluster++;
        }
    }
}

/**
 * Generate file content for given block address
 *
 * @param buf   Destination for generated data (512 bytes)
 * @param block Block offset relative to the first block of data area (0-...)
 */
static void Ramdiski_GetFile(uint8_t *buf, uint32_t block)
{
    int id;
    uint16_t cluster = block / SECTORS_PER_CLUSTER + 2;
    uint32_t offset;

    for (id = 0; id < RAMDISK_MAX_FILES; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            return;
        }
        if (cluster < ramdiski_files[id].cluster ||
                cluster > ramdiski_files[id].cluster + ramdiski_files[id].size/CLUSTER_SIZE) {
            continue;
        }

        offset = block - (ramdiski_files[id].cluster - 2)*SECTORS_PER_CLUSTER;
        offset *= SECTOR_SIZE;
        if (offset >= ramdiski_files[id].size) {
            continue;
        }

        if (ramdiski_files[id].read != NULL) {
            uint32_t size = ramdiski_files[id].size - offset;
            if (size > SECTOR_SIZE) {
                size = SECTOR_SIZE;
            }
            ramdiski_files[id].read(offset, buf, size);
        } else {
            Ramdiski_ReadTextFile(id, offset, buf);
        }
        return;
    }
}

/**
 * Writing to data area
 *
 * If data are written outside existing files, user function is called
 * with data and offset from first empty cluster
 *
 * @param buf   Data to be written
 * @param block Block offset relative to the first block of data area (0-...)
 */
static void Ramdiski_WriteData(uint8_t *buf, uint32_t block)
{
    //TODO
}

int Ramdisk_Read(uint32_t lba, uint8_t *buf)
{
    ASSERT_NOT(buf == NULL);

    if (lba == 0) {
        memset(buf, 0, SECTOR_SIZE);
        memcpy(buf, fat16i_boot_sector, sizeof(fat16i_boot_sector));
        if (ramdiski_info.sectors_count < 65535) {
            /* Small number of sectors */
            Ramdiski_To2Bytes(ramdiski_info.sectors_count, &buf[0x13]);
        } else {
            /* Large number of sectors */
            Ramdiski_To4Bytes(ramdiski_info.sectors_count, &buf[0x20]);
        }
        /* Sectors per FAT */
        Ramdiski_To2Bytes(ramdiski_info.fat_sectors, &buf[0x16]);
        /* Disk name */
        memcpy(&buf[0x2b], ramdiski_info.name, 11);
        /* boot sector signature */
        buf[SECTOR_SIZE - 2] = 0x55;
        buf[SECTOR_SIZE - 1] = 0xAA;
    } else if (lba >= FAT1_START_SECTOR && lba < FAT2_START_SECTOR) {
        Ramdiski_GetFAT16(buf, lba - FAT1_START_SECTOR);
    } else if (lba >= FAT2_START_SECTOR && lba < ROOT_START_SECTOR) {
        Ramdiski_GetFAT16(buf, lba - FAT2_START_SECTOR);
    } else if (lba >= ROOT_START_SECTOR && lba < DATA_START_SECTOR) {
        Ramdiski_GetRootDirectory(buf, lba - ROOT_START_SECTOR);
    } else if (lba >= DATA_START_SECTOR) {
        Ramdiski_GetFile(buf, lba - DATA_START_SECTOR);
    }
    return 0;
}

int Ramdisk_Write(uint32_t lba, const uint8_t *buf)
{
    ASSERT_NOT(buf == NULL);

    if (lba >= FAT1_START_SECTOR && lba < FAT2_START_SECTOR) {
        /* FAT writes are ignored */
    } else if (lba >= FAT2_START_SECTOR && lba < ROOT_START_SECTOR) {
        /* FAT writes are ignored */
    } else if (lba >= ROOT_START_SECTOR && lba < DATA_START_SECTOR) {
        Ramdiski_WriteRootDirectory(buf, lba - ROOT_START_SECTOR);
    } else if (lba >= DATA_START_SECTOR) {
        Ramdiski_WriteData(buf, lba - DATA_START_SECTOR);
    }
    return 0;
}

bool Ramdisk_AddFile(const char *filename, const char *extension, time_t time,
        size_t size, ramdisk_read_t read)
{
    ASSERT_NOT(read == NULL);
    return Ramdiski_AddFile(filename, extension, time, size, read, NULL);
}

bool Ramdisk_AddTextFile(const char *filename, const char *extension,
        time_t time, const char *text)
{
    ASSERT_NOT(text == NULL);
    return Ramdiski_AddFile(filename, extension, time, strlen(text), NULL, text);
}

void Ramdisk_Clear(void)
{
    memset(ramdiski_files, 0x00, sizeof(ramdiski_files));
}

uint32_t Ramdisk_GetSectors(void)
{
    return ramdiski_info.sectors_count;
}

void Ramdisk_Init(size_t size, const char *name)
{
    uint32_t clusters;

    ASSERT_NOT(size/CLUSTER_SIZE >= 65524);
    /* Increase size to achieve minimal required cluster count for fat 16 */
    if (size < FAT16_MIN_CLUSTERS*CLUSTER_SIZE) {
        size = FAT16_MIN_CLUSTERS*CLUSTER_SIZE;
    }

    ramdiski_info.sectors_count = ceil_div(size, SECTOR_SIZE);
    clusters = ceil_div(ramdiski_info.sectors_count, SECTORS_PER_CLUSTER) + 2;
    ramdiski_info.fat_sectors = ceil_div(clusters, SECTOR_SIZE/2);
    strncpy(ramdiski_info.name, name, sizeof(ramdiski_info.name));
    for (uint8_t i = strlen(name); i < sizeof(ramdiski_info.name); i++) {
        ramdiski_info.name[i] = ' ';
    }
}

/** @} */
