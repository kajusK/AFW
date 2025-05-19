/**
 * @file    drivers/sd_spi.c
 * @brief   SD card driver (over SPI)
 *
 * http://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf
 * http://elm-chan.org/docs/mmc/mmc_e.html
 * https://www.sdcard.org/downloads/pls/
 *
 * Based on generic example from fatfs sources
 */

#include <types.h>
#include "hal/io.h"
#include "hal/spi.h"
#include "utils/time.h"
#include "sd_spi.h"

/* MMC/SD command (SPI mode) */
#define CMD0   (0)         /* GO_IDLE_STATE */
#define CMD1   (1)         /* SEND_OP_COND */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8   (8)         /* SEND_IF_COND */
#define CMD9   (9)         /* SEND_CSD */
#define CMD10  (10)        /* SEND_CID */
#define CMD12  (12)        /* STOP_TRANSMISSION */
#define CMD13  (13)        /* SEND_STATUS */
#define ACMD13 (0x80 + 13) /* SD_STATUS (SDC) */
#define CMD16  (16)        /* SET_BLOCKLEN */
#define CMD17  (17)        /* READ_SINGLE_BLOCK */
#define CMD18  (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)        /* SET_BLOCK_COUNT */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)        /* WRITE_BLOCK */
#define CMD25  (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32  (32)        /* ERASE_ER_BLK_START */
#define CMD33  (33)        /* ERASE_ER_BLK_END */
#define CMD38  (38)        /* ERASE */
#define CMD55  (55)        /* APP_CMD */
#define CMD58  (58)        /* READ_OCR */

/* MMC get type types */
#define CT_MMC   0x01              /* MMC ver 3 */
#define CT_SD1   0x02              /* SD ver 1 */
#define CT_SD2   0x04              /* SD ver 2 */
#define CT_SDC   (CT_SD1 | CT_SD2) /* SD */
#define CT_BLOCK 0x08              /* Block addressing */

#define timed_out(timeout) ((millis() - start_ts) > (timeout))

/**
 * Wait for card to become ready
 *
 * @param desc  Card descriptor
 * @return true when card is ready, false if timed out (500 ms)
 */
static bool waitReady(const sdspi_desc_t *desc)
{
    uint8_t resp;
    uint32_t start_ts = millis();

    do {
        resp = SPId_Transceive(desc->device, 0xff);
    } while (!timed_out(500) && resp != 0xff);

    if (resp == 0xff) {
        return true;
    }
    return false;
}

/**
 * Deselect card after communication
 * @param desc  Card descriptor
 */
static void deselect(const sdspi_desc_t *desc)
{
    IOd_SetLine(desc->cs_port, desc->cs_pad, true);
    // dummy byte
    (void)SPId_Transceive(desc->device, 0xff);
}

/**
 * Select the card and prepare for communication
 *
 * @param desc  Card descriptor
 * @return true when card is ready, false if timed out (500 ms)
 */
static bool select(const sdspi_desc_t *desc)
{
    IOd_SetLine(desc->cs_port, desc->cs_pad, false);
    (void)SPId_Transceive(desc->device, 0xff); // Dummy clock (force DO enabled)
    if (!waitReady(desc)) {
        deselect(desc);
        return false;
    }
    return true;
}

/**
 * Receive a data block from the card
 *
 * @param desc  Card descriptor
 * @param buf	Data buffer
 * @param bytes	Amount of bytes to receive (multiples of 4)
 *
 * @return Successfulness of the operation
 */
static bool readData(const sdspi_desc_t *desc, uint8_t *buf, uint32_t bytes)
{
    uint8_t resp;
    uint32_t start_ts = millis();

    do {
        resp = SPId_Transceive(desc->device, 0xff);
    } while (!timed_out(100) && resp == 0xff);

    if (resp != 0xFE) {
        return false; // data token not valid -> error
    }

    SPId_Receive(desc->device, buf, bytes);
    /* discard CRC */
    (void)SPId_Transceive(desc->device, 0xff);
    (void)SPId_Transceive(desc->device, 0xff);

    return true;
}

/**
 * Write a data block to the card
 *
 * @param desc  Card descriptor
 * @param buff  Data buffer (for 0xfd token), 512 bytes
 * @param token Command token
 *
 * @return Successfulness of the operation
 */
static bool writeData(const sdspi_desc_t *desc, const uint8_t *buff, uint8_t token)
{
    uint8_t resp;

    if (!waitReady(desc)) {
        return false;
    }

    SPId_Transceive(desc->device, token);
    // Data token
    if (token != 0xFD) {
        SPId_Send(desc->device, buff, 512);
        // dummy crc
        SPId_Transceive(desc->device, 0xff);
        SPId_Transceive(desc->device, 0xff);

        resp = SPId_Transceive(desc->device, 0xff);
        if ((resp & 0x1F) != 0x05) {
            // not accepted
            return false;
        }
    }

    return true;
}

/**
 * Write a data block to the card
 *
 * @param desc  Card descriptor
 * @param cmd	Command to be written
 * @param arg   Command argument
 *
 * @return Command response (bit 7 in 1 means error)
 */
static uint8_t writeCmd(const sdspi_desc_t *desc, uint8_t cmd, uint32_t arg)
{
    uint8_t resp, attempts;
    uint8_t buf[6];

    if (cmd & 0x80) { // ACMD<n> is the command sequence of CMD55-CMD<n>
        cmd &= 0x7F;
        resp = writeCmd(desc, CMD55, 0);
        if (resp > 1) {
            return resp;
        }
    }

    // Select the card and wait for ready except to stop multiple block read
    if (cmd != CMD12) {
        deselect(desc);
        if (!select(desc)) {
            return 0xFF;
        }
    }

    buf[0] = 0x40 | cmd; // Start + Command index
    buf[1] = (uint8_t)(arg >> 24);
    buf[2] = (uint8_t)(arg >> 16);
    buf[3] = (uint8_t)(arg >> 8);
    buf[4] = (uint8_t)arg;

    buf[5] = 0x01; // Dummy CRC + Stop
    if (cmd == CMD0) {
        buf[5] = 0x95; // valid CRC for CMD0(0)
    } else if (cmd == CMD8) {
        buf[5] = 0x87; // valid CRC for CMD8(0x1AA)
    }
    SPId_Send(desc->device, buf, 6);

    // Get command response
    if (cmd == CMD12) {
        // Skip a stuff byte
        (void)SPId_Transceive(desc->device, 0xff);
    }

    // try to get a valid response
    attempts = 10;
    do {
        resp = SPId_Transceive(desc->device, 0xff);
    } while ((resp & 0x80) && --attempts);

    return resp;
}

bool SDSPI_InitCard(sdspi_desc_t *desc)
{
    spid_prescaler_t prescaler;
    uint32_t start_ts;
    uint8_t type = 0;
    uint8_t cmd, buf[4];

    // SPI clock must be lower than 400 kHz in init mode
    prescaler = SPId_GetPrescaler(desc->device);
    SPId_SetPrescaler(desc->device, SPID_PRESC_256);

    IOd_SetLine(desc->cs_port, desc->cs_pad, true);
    // Apply 80 dummy clocks to the card gets ready to receive commands
    for (uint8_t i = 0; i < 10; i++) {
        SPId_Transceive(desc->device, 0xff);
    }

    if (writeCmd(desc, CMD0, 0) == 1) { // Enter Idle state
        start_ts = millis();
        if (writeCmd(desc, CMD8, 0x1AA) == 1) {     // SDv2?
            SPId_Receive(desc->device, buf, 4);     // Get trailing return value of R7 response
            if (buf[2] == 0x01 && buf[3] == 0xAA) { // The card can work at vdd range of 2.7-3.6V
                // Wait for leaving idle state (ACMD41 with HCS bit)
                while (!timed_out(1000) && writeCmd(desc, ACMD41, 1UL << 30)) {
                    ;
                }

                // Check CCS bit in the OCR
                if (!timed_out(1000) && writeCmd(desc, CMD58, 0) == 0) {
                    SPId_Receive(desc->device, buf, 4);
                    // SDv2
                    type = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        } else { // SDv1 or MMCv3
            if (writeCmd(desc, ACMD41, 0) <= 1) {
                type = CT_SD1;
                cmd = ACMD41; // SDv1
            } else {
                type = CT_MMC;
                cmd = CMD1; // MMCv3
            }
            // Wait for leaving idle state
            while (!timed_out(1000) && writeCmd(desc, cmd, 0)) {
                ;
            }

            // Set R/W block length to 512
            if (timed_out(1000) || writeCmd(desc, CMD16, 512) != 0) {
                type = 0;
            }
        }
    }
    desc->card_type = type;
    deselect(desc);

    SPId_SetPrescaler(desc->device, prescaler);
    return type != 0;
}

bool SDSPI_ReadSector(sdspi_desc_t *desc, uint8_t *buff, uint32_t sector, uint32_t count)
{
    uint8_t cmd;

    if (!(desc->card_type & CT_BLOCK)) {
        sector *= 512; // Convert LBA to byte address
    }

    // READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK
    cmd = count > 1 ? CMD18 : CMD17;

    if (writeCmd(desc, cmd, sector) == 0) {
        do {
            if (!readData(desc, buff, 512)) {
                break;
            }
            buff += 512;
        } while (--count);

        if (cmd == CMD18) {
            writeCmd(desc, CMD12, 0); // STOP_TRANSMISSION
        }
    }
    deselect(desc);
    return count == 0;
}

bool SDSPI_WriteSector(sdspi_desc_t *desc, const uint8_t *buff, uint32_t sector, uint32_t count)
{
    if (!(desc->card_type & CT_BLOCK)) {
        sector *= 512; // Convert LBA to byte address if needed
    }

    if (count == 1) { // Single block write
        if ((writeCmd(desc, CMD24, sector) == 0) && writeData(desc, buff, 0xFE)) {
            count = 0;
        }
    } else { // Multiple block write
        if (desc->card_type & CT_SDC) {
            writeCmd(desc, ACMD23, count);
        }
        if (writeCmd(desc, CMD25, sector) == 0) { // WRITE_MULTIPLE_BLOCK
            do {
                if (!writeData(desc, buff, 0xFC)) {
                    break;
                }
                buff += 512;
            } while (--count);
            if (!writeData(desc, 0, 0xFD)) {
                // STOP_TRAN token
                count = 1;
            }
        }
    }
    deselect(desc);
    return count == 0;
}

bool SDSPI_Sync(sdspi_desc_t *desc)
{
    bool ret = select(desc);
    deselect(desc);
    return ret;
}

uint32_t SDSPI_GetSectorsCount(sdspi_desc_t *desc)
{
    uint8_t csd[16];
    uint32_t sectors = 0;

    if ((writeCmd(desc, CMD9, 0) == 0) && readData(desc, csd, 16)) {
        if ((csd[0] >> 6) == 1) {
            // SDC ver 2.00
            sectors = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
            sectors <<= 10;
        } else {
            // SDC ver 1.XX or MMC
            uint8_t n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
            sectors = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
            sectors <<= (n - 9);
        }
    }
    deselect(desc);
    return sectors;
}

void SDSPI_SetInserted(sdspi_desc_t *desc, bool present)
{
    desc->present = present;
    if (!present) {
        desc->card_type = 0;
    }
}

bool SDSPI_IsInserted(sdspi_desc_t *desc)
{
    return desc->present;
}

bool SDSPI_IsInitialized(sdspi_desc_t *desc)
{
    return desc->card_type != 0;
}

void SDSPI_Init(sdspi_desc_t *desc, uint8_t spi, uint32_t cs_port, uint8_t cs_pad)
{
    desc->device = spi;
    desc->cs_port = cs_port;
    desc->cs_pad = cs_pad;
    desc->present = false;
    desc->card_type = 0;
}
