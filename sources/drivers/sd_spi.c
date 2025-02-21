/**
 * @file    drivers/sd_spi.c
 * @brief   SD card driver (over SPI) for FatFS library
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
#include "ff.h"			/* FatFs library header */
#include "diskio.h"		/* FatFs lower layer API */
#include "sd_spi.h"

/* MMC/SD command (SPI mode) */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define CMD13	(13)		/* SEND_STATUS */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* MMC get type types */
#define CT_MMC          0x01            /* MMC ver 3 */
#define CT_SD1          0x02            /* SD ver 1 */
#define CT_SD2          0x04            /* SD ver 2 */
#define CT_SDC          (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK        0x08            /* Block addressing */

#define timed_out(timeout) ((millis() - start_ts) > (timeout))

typedef struct {
	uint8_t device;		/**< SPI device */
	uint32_t cs_port;	/**< Port of CS pin */
	uint32_t cs_pad;	/**< Pad of CS pin */
	DSTATUS status;		/**< Disk status */
	BYTE card_type;		/**< Type of the card: 0: MMC, 1: SDv1, 2: Block addressing */
} sd_card_t;

/**
 * Wait for card to become ready
 *
 * @param card	Card descriptor
 * @return true when card is ready, false if timed out (500 ms)
 */
static bool waitReady(const sd_card_t *card)
{
	uint8_t resp;
	uint32_t start_ts = millis();

	do {
		resp = SPId_Transceive(card->device, 0xff);
	} while (!timed_out(500) && resp != 0xff);

	if (resp == 0xff) {
		return true;
	}
	return false;
}

/**
 * Deselect card after communication
 * @param card	Card descriptor
 */
static void deselect(const sd_card_t *card)
{
	IOd_SetLine(card->cs_port, card->cs_pad, true); // cs high
	// dummy byte
	(void) SPId_Transceive(card->device, 0xff);
}

/**
 * Select the card and prepare for communication
 *
 * @param card	Card descriptor
 * @return true when card is ready, false if timed out (500 ms)
 */
static bool select(const sd_card_t *card)
{
	IOd_SetLine(card->cs_port, card->cs_pad, false); // CS low
	(void) SPId_Transceive(card->device, 0xff); // Dummy clock (force DO enabled)
	if (!waitReady(card)) {
		deselect(card);
		return false;
	}
	return true;
}

/**
 * Receive a data block from the card
 *
 * @param card	Card descriptor
 * @param buf	Data buffer
 * @param bytes	Amount of bytes to receive
 *
 * @return Successfulness of the operation
 */
static bool readData(const sd_card_t *card, BYTE *buf, UINT bytes)
{
	uint8_t resp;
	uint32_t start_ts = millis();

	do {
		resp = SPId_Transceive(card->device, 0xff);
	} while (!timed_out(100) && resp == 0xff);

	if (resp != 0xFE) {
		return false;	// data token not valid -> error
	}

	SPId_Receive(card->device, buf, bytes);
	/* discard CRC */
	(void)SPId_Transceive(card->device, 0xff);
	(void)SPId_Transceive(card->device, 0xff);

	return true;
}

/**
 * Write a data block to the card
 *
 * @param card	Card descriptor
 * @param buf	Data buffer (for 0xfd token), 512 bytes
 * @param token Command token
 *
 * @return Successfulness of the operation
 */
static bool writeData(const sd_card_t *card, const BYTE *buff, BYTE token)
{
	uint8_t resp;

	if (!waitReady(card)) {
		return false;
	}

	SPId_Transceive(card->device, token);
	// Data token
	if (token != 0xFD) {
		SPId_Send(card->device, buff, 512);
		// dummy crc
		SPId_Transceive(card->device, 0xff);
		SPId_Transceive(card->device, 0xff);

		resp = SPId_Transceive(card->device, 0xff);
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
 * @param card	Card descriptor
 * @param cmd	Command to be written
 * @param arg   Command argument
 *
 * @return Command response (bit 7 in 1 means error)
 */
static BYTE writeCmd(const sd_card_t *card, BYTE cmd, DWORD arg)
{
	BYTE n, resp, attempts;
	BYTE buf[6];

	if (cmd & 0x80) {	// ACMD<n> is the command sequence of CMD55-CMD<n>
		cmd &= 0x7F;
		resp = writeCmd(card, CMD55, 0);
		if (resp > 1) {
			return resp;
		}
	}

	// Select the card and wait for ready except to stop multiple block read
	if (cmd != CMD12) {
		deselect(card);
		if (!select(card)) {
			return 0xFF;
		}
	}

	buf[0] = 0x40 | cmd;			// Start + Command index
	buf[1] = (BYTE)(arg >> 24);
	buf[2] = (BYTE)(arg >> 16);
	buf[3] = (BYTE)(arg >> 8);
	buf[4] = (BYTE)arg;

	n = 0x01;			// Dummy CRC + Stop
	if (cmd == CMD0) {
		n = 0x95;		// valid CRC for CMD0(0)
	} else if (cmd == CMD8) {
		n = 0x87;		// valid CRC for CMD8(0x1AA)
	}
	buf[5] = n;
	SPId_Send(card->device, buf, 6);

	// Get command response
	if (cmd == CMD12) {
		// Skip a stuff byte
		(void) SPId_Transceive(card->device, 0xff);
	}

	// try to get a valid response
	attempts = 10;
	do {
		resp = SPId_Transceive(card->device, 0xff);
	} while ((resp & 0x80) && --attempts);

	return resp;
}

/**
 * Get card descriptor from driver ID
 *
 * @param drv	FatFS Drive ID, only 0 is available
 * @return  Descriptor or NULL if drive not available
 */
static sd_card_t *getDrive(BYTE drv)
{
	static sd_card_t descriptor;

	if (drv == 0) {
		return &descriptor;
	}
	return NULL;
}

/**
 * FatFs get disk status callback
 *
 * @param drv	Drive ID
 */
DSTATUS disk_status(BYTE drv)
{
	const sd_card_t *card = getDrive(drv);
	if (card == NULL) {
		return STA_NOINIT;
	}
	return card->status;
}

/**
 * Initialize device and put it into ready state
 *
 * @param drv	Drive ID
 */
DSTATUS disk_initialize(BYTE drv)
{
	sd_card_t *card = getDrive(drv);
	spid_prescaler_t prescaler;
	uint32_t start_ts;
	BYTE type;
	BYTE cmd, buf[4];

	if (card == NULL) {
		return RES_NOTRDY;
	}

	// SPI clock must be lower than 400 kHz in init mode
	prescaler = SPId_GetPrescaler(card->device);
	SPId_SetPrescaler(card->device, SPID_PRESC_256);

	IOd_SetLine(card->cs_port, card->cs_pad, true);

	// Apply 80 dummy clocks to the card gets ready to receive commands
	for (uint8_t i = 0; i < 10; i++) {
		SPId_Transceive(card->device, 0xff);
	}

	type = 0;
	if (writeCmd(card, CMD0, 0) == 1) { // Enter Idle state
		start_ts = millis();
		if (writeCmd(card, CMD8, 0x1AA) == 1) { // SDv2?
			SPId_Receive(card->device, buf, 4); // Get trailing return value of R7 response
			if (buf[2] == 0x01 && buf[3] == 0xAA) { // The card can work at vdd range of 2.7-3.6V
				// Wait for leaving idle state (ACMD41 with HCS bit)
				while (!timed_out(1000) && writeCmd(card, ACMD41, 1UL << 30)) {
					;
				}

				// Check CCS bit in the OCR
				if (!timed_out(1000) && writeCmd(card, CMD58, 0) == 0) {
					SPId_Receive(card->device, buf, 4);
					// SDv2
					type = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		} else { // SDv1 or MMCv3
			if (writeCmd(card, ACMD41, 0) <= 1) {
				type = CT_SD1;
				cmd = ACMD41;	// SDv1
			} else {
				type = CT_MMC;
				cmd = CMD1;	// MMCv3
			}
			// Wait for leaving idle state
			while (!timed_out(1000) && writeCmd(card, cmd, 0)) {
				;
			}

			// Set R/W block length to 512
			if (timed_out(1000) || writeCmd(card, CMD16, 512) != 0) {
				type = 0;
			}
		}
	}
	card->card_type = type;
	deselect(card);

	card->status = type ? 0 : STA_NOINIT;

	SPId_SetPrescaler(card->device, prescaler);
	return card->status;
}

/**
 * Read data from storage device
 *
 * @param drv		Drive ID
 * @param buff		Read data buffer
 * @param sector	Start sector number
 * @param count		Number of sectors to read
 */
DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
	BYTE cmd;
	const sd_card_t *card = getDrive(drv);

	if (disk_status(drv) & STA_NOINIT) {
		return RES_NOTRDY;
	}

	if (!(card->card_type & CT_BLOCK)) {
		sector *= 512;	// Convert LBA to byte address
	}

	// READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK
	cmd = count > 1 ? CMD18 : CMD17;

	if (writeCmd(card, cmd, sector) == 0) {
		do {
			if (!readData(card, buff, 512)) {
				break;
			}
			buff += 512;
		} while (--count);

		if (cmd == CMD18) {
			writeCmd(card, CMD12, 0);	// STOP_TRANSMISSION
		}
	}
	deselect(card);

	return count ? RES_ERROR : RES_OK;
}

/**
 * Write data to storage device
 *
 * @param drv		Drive ID
 * @param buff		Write data buffer
 * @param sector	Start sector number
 * @param count		Number of sectors to write
 */
DRESULT disk_write(BYTE drv, const BYTE *buff,	DWORD sector, UINT count)
{
	const sd_card_t *card = getDrive(drv);
	if (disk_status(drv) & STA_NOINIT) {
		return RES_NOTRDY;
	}

	if (!(card->card_type & CT_BLOCK)) {
		sector *= 512;	// Convert LBA to byte address if needed
	}

	if (count == 1) {	// Single block write
		if ((writeCmd(card, CMD24, sector) == 0) && writeData(card, buff, 0xFE)) {
			count = 0;
		}
	} else { // Multiple block write
		if (card->card_type & CT_SDC) {
			writeCmd(card, ACMD23, count);
		}
		if (writeCmd(card, CMD25, sector) == 0) { // WRITE_MULTIPLE_BLOCK
			do {
				if (!writeData(card, buff, 0xFC)) {
					break;
				}
				buff += 512;
			} while (--count);
			if (!writeData(card, 0, 0xFD)) {
				// STOP_TRAN token
				count = 1;
			}
		}
	}
	deselect(card);

	return count ? RES_ERROR : RES_OK;
}

/**
 * Control device specific features
 *
 * @param drv		Drive ID
 * @param ctrl		Control command code
 * @param buff		Parameter and data buffer
 */
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
	DRESULT res = RES_ERROR;
	const sd_card_t *card = getDrive(drv);

	if (disk_status(drv) & STA_NOINIT) {
		return RES_NOTRDY;
	}

	res = RES_ERROR;
	switch (ctrl) {
		// Make sure that no pending write process
		case CTRL_SYNC:
			if (select(card)) {
				res = RES_OK;
			}
			break;

		// Get number of sectors on the disk (DWORD)
		case GET_SECTOR_COUNT: {
			BYTE csd[16];

			if ((writeCmd(card, CMD9, 0) == 0) && readData(card, csd, 16)) {
				DWORD sectors;

				if ((csd[0] >> 6) == 1) {
					// SDC ver 2.00
					sectors = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
					*(DWORD*)buff = sectors << 10;
				} else {
					// SDC ver 1.XX or MMC
					BYTE n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					sectors = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = sectors << (n - 9);
				}
				res = RES_OK;
			}
			break;
		}

		// Get erase block size in unit of sector (DWORD)
		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 128;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}

	deselect(card);
	return res;
}

/**
 * Get current timestamp
 *
 * Override in other place in code to provide correct data, only a dummy
 * implementation is here.
 *
 * A valid data must be returned, 1.1.2024 is used here.
 *
 * @see http://elm-chan.org/fsw/ff/doc/fattime.html
 */
DWORD __attribute__((weak)) get_fattime(void)
{
	DWORD timestamp = 0;
	timestamp |= 1 << 16; // day 1-12
	timestamp |= 1 << 21;  // month 1-12
	timestamp |= (2024 - 1980) << 25; // years since 1980

	return timestamp;
}

void SdCard_SetInsertion(bool present)
{
	sd_card_t *card = getDrive(0);
	if (!present) {
		card->status = STA_NOINIT;
	}
}

void SdCard_Init(uint8_t spi, uint32_t cs_port, uint8_t cs_pad)
{
	sd_card_t *card = getDrive(0);

	card->device = spi;
	card->cs_port = cs_port;
	card->cs_pad = cs_pad;
	card->status = STA_NOINIT;
	card->card_type = 0;
}
