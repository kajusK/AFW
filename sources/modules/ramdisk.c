/**
 * @file    modules/ramdisk.h
 * @brief   RAMdisk emulation with on the fly file creation, uses FAT16
 *
 * FAT16 description http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html
 * http://www.tavi.co.uk/phobos/fat.html
 */

#include <string.h>
#include <types.h>
#include "utils/math.h"
#include "ramdisk.h"

/** 1, 2, 4, 8, 16, 32, 128 */
#define SECTORS_PER_CLUSTER 8
/** Files/subdirs in root directory */
#define ROOT_ENTRIES 512U

/* Do not change below defines */
/* bytes in sector */
#define SECTOR_SIZE 512
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

/** Directory entry FAT16 structure */
typedef struct {
    char filename[8];       /**< Filename, zero padded, [0]=0x00 stop search */
    char extension[3];
    uint8_t attribute;
    uint8_t reserved;
    uint8_t creation;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t reserved2;
    uint16_t last_write_time;
    uint16_t last_write_date;
    uint16_t start_cluster;
    uint32_t file_size;
} __attribute__((__packed__)) fat_dir_entry_t;

/** Type for virtual files */
typedef struct {
    uint8_t name[8];       /**< File name (padded with spaces) */
    uint8_t extension[3];  /**< File extension */
    uint8_t time[2];        /**< Time created in fat format */
    uint8_t date[2];        /**< Date created in fat format */
    uint8_t attr;           /**< File attributes */
    uint32_t size;          /**< File size in bytes */
    uint16_t cluster;       /**< First cluster of the file */
    ramdisk_read_t read;    /**< Function called upon read requests or NULL */
    const char *content;    /**< Text content of the file when read is NULL */
} ramdisk_file_t;

/** Ramdisk parameters */
typedef struct {
    uint32_t sectors_count; /**< Size of volume in sectors */
    uint16_t fat_sectors;   /**< Size of fat table in sectors */
    char name[11];          /**< Volume label */
} ramdisk_info_t;

/** Info about a new file being written to ramdisk */
typedef struct {
    char name[9];       /**< File name including termination character */
    char extension[4];  /**< File extension including termination character */
    uint32_t size;      /**< File size in bytes */
    uint16_t cluster;   /**< Start cluster */
} ramdisk_write_file_t;

/** Files shown in ramdisk, if name starts with 0, file is ignored */
static ramdisk_file_t ramdiski_files[RAMDISK_MAX_FILES];

/** Parameters of the ramdisk that are calculated during runtime */
static ramdisk_info_t ramdiski_info;

/** Write file callback */
static ramdisk_write_file_cb_t ramdiski_write_file_cb;

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
    0x80, /* Drive number */
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
 * @return  id of the file added or -1 if failed
 */
static int Ramdiski_AddFile(const char *filename, const char *extension,
        time_t time, size_t size, ramdisk_read_t read, const char *content)
{
    uint16_t id;
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
        return -1;
    }

    /* check if there's enough clusters for the file */
    if (cluster + size/CLUSTER_SIZE >= 0xffef) {
        return -1;
    }

    /* Set filename, pad with spaces */
    strncpy((char *)ramdiski_files[id].name, filename, 8);
    for (size_t i = 7; i >= strlen(filename); i--) {
        ramdiski_files[id].name[i] = ' ';
    }

    /* Set file extension, pad with spaces */
    strncpy((char *)ramdiski_files[id].extension, extension, 3);
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
    ramdiski_files[id].attr = 0x21; /* archive, read only */
    ramdiski_files[id].read = read;
    if (read == NULL) {
        ramdiski_files[id].content = content;
    }
    return id;
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
    uint16_t id;
    uint16_t skip;  /* amount of dir entries to skip */
    fat_dir_entry_t *entry = (fat_dir_entry_t *) buf;

    memset(buf, 0x00, SECTOR_SIZE);
    /* First entry in the root directory is the volume label */
    if (block == 0) {
        memcpy(buf, ramdiski_info.name, 11);
        buf[11] = 0x08;     /* volume label attribute */
        entry += 1;
        skip = 0;
    } else {
        /*
         * Amount of files already contained in previous blocks
         * (-1 for volume label)
         */
        skip = (SECTOR_SIZE/DIR_ENTRY_SIZE)*block - 1;
    }

    /* Each record is 32 bytes wide, so it aligns to 512B sectors nicely */
    for (id = 0; id < RAMDISK_MAX_FILES && ((uint8_t *)entry - buf) < SECTOR_SIZE; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            break;
        }
        /* skip files for previous blocks */
        if (skip != 0) {
            skip--;
            continue;
        }

        memcpy(entry->filename, ramdiski_files[id].name, 8);
        memcpy(entry->extension, ramdiski_files[id].extension, 3);
        entry->attribute = ramdiski_files[id].attr;
        memcpy(&entry->last_write_time, ramdiski_files[id].time, 2);
        memcpy(&entry->last_write_date, ramdiski_files[id].date, 2);
        Ramdiski_To2Bytes(ramdiski_files[id].cluster, (uint8_t *) &entry->start_cluster);
        Ramdiski_To4Bytes(ramdiski_files[id].size, (uint8_t *) &entry->file_size);
        entry++;
    }
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
            break;
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
    uint16_t id;
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
 * As the data are written before root directory file entry, assume all
 * data written in first empty cluster belong to a file being written.
 *
 * @param buf   Data to be written
 * @param block Block offset relative to the first block of data area (0-...)
 */
static void Ramdiski_WriteData(const uint8_t *buf, uint32_t block)
{
    uint16_t cur_cluster = block / SECTORS_PER_CLUSTER + 2;
    uint16_t last_cluster = 0;
    uint16_t end_cluster;
    uint16_t id;
    uint32_t offset;

    /* Calculate cluster where the last virtual file ends */
    for (id = 0; id < RAMDISK_MAX_FILES; id++) {
        if (ramdiski_files[id].name[0] == 0x00) {
            break;
        }

        end_cluster = ramdiski_files[id].cluster + ramdiski_files[id].size/CLUSTER_SIZE;
        if (end_cluster > last_cluster) {
            last_cluster = end_cluster;
        }
    }

    if (last_cluster >= cur_cluster) {
        /* Trying to write to virtual files area */
        return;
    }

    offset = block - (last_cluster + 1 - 2)*SECTORS_PER_CLUSTER;
    offset *= SECTOR_SIZE;

    if (ramdiski_write_file_cb != NULL) {
        ramdiski_write_file_cb(buf, SECTOR_SIZE, offset);
    }
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

    if (lba == 0) {
        /* Boot sector writes are ignored */
    } else if (lba >= FAT1_START_SECTOR && lba < FAT2_START_SECTOR) {
        /* FAT writes are ignored */
    } else if (lba >= FAT2_START_SECTOR && lba < ROOT_START_SECTOR) {
        /* FAT writes are ignored */
    } else if (lba >= ROOT_START_SECTOR && lba < DATA_START_SECTOR) {
        /* Root directory writes are ignored */
    } else if (lba >= DATA_START_SECTOR) {
        Ramdiski_WriteData(buf, lba - DATA_START_SECTOR);
    }
    return 0;
}

int Ramdisk_AddFile(const char *filename, const char *extension, time_t time,
        size_t size, ramdisk_read_t read)
{
    ASSERT_NOT(read == NULL);
    return Ramdiski_AddFile(filename, extension, time, size, read, NULL);
}

int Ramdisk_AddTextFile(const char *filename, const char *extension,
        time_t time, const char *text)
{
    ASSERT_NOT(text == NULL);
    return Ramdiski_AddFile(filename, extension, time, strlen(text), NULL, text);
}

bool Ramdisk_RenameFile(int handle, const char *filename, const char *extension)
{
    if (handle >= RAMDISK_MAX_FILES || ramdiski_files[handle].name[0] == 0x00) {
        return false;
    }

    /* Set filename, pad with spaces */
    strncpy((char *)ramdiski_files[handle].name, filename, 8);
    for (size_t i = 7; i >= strlen(filename); i--) {
        ramdiski_files[handle].name[i] = ' ';
    }

    /* Set file extension, pad with spaces */
    strncpy((char *)ramdiski_files[handle].extension, extension, 3);
    for (size_t i = 2; i >= strlen(extension); i--) {
        ramdiski_files[handle].extension[i] = ' ';
    }
    return true;
}

void Ramdisk_Clear(void)
{
    memset(ramdiski_files, 0x00, sizeof(ramdiski_files));
}

uint32_t Ramdisk_GetSectors(void)
{
    return ramdiski_info.sectors_count;
}

void Ramdisk_RegisterWriteCb(ramdisk_write_file_cb_t cb)
{
    ramdiski_write_file_cb = cb;
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

    /* set volume label, pad with spaces */
    strncpy(ramdiski_info.name, name, sizeof(ramdiski_info.name));
    for (uint8_t i = strlen(name); i < sizeof(ramdiski_info.name); i++) {
        ramdiski_info.name[i] = ' ';
    }
}
