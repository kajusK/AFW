/**
 * @file    modules/msc.h
 * @brief   USB Mass storage class implementation
 *
 * Based on http://libopencm3.org/docs/latest/stm32f1/html/usb__msc_8c_source.html
 *
 * Specifications:
 * https://www.usb.org/sites/default/files/usbmassbulk_10.pdf
 * https://www.seagate.com/files/staticfiles/support/docs/manual/Interface%20manuals/100293068j.pdf
 */

#include <stdlib.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/msc.h>
#include <modules/msc.h>

/* Command Block Wrapper */
#define CBW_SIGNATURE               0x43425355

/* Command Status Wrapper */
#define CSW_SIGNATURE               0x53425355
#define CSW_STATUS_SUCCESS          0
#define CSW_STATUS_FAILED           1
#define CSW_STATUS_PHASE_ERROR      2

/* Implemented SCSI Commands */
#define SCSI_TEST_UNIT_READY        0x00
#define SCSI_REQUEST_SENSE          0x03
#define SCSI_FORMAT_UNIT            0x04
#define SCSI_READ_6                 0x08
#define SCSI_WRITE_6                0x0A
#define SCSI_INQUIRY                0x12
#define SCSI_MODE_SENSE_6           0x1A
#define SCSI_SEND_DIAGNOSTIC        0x1D
#define SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL   0x1E
#define SCSI_READ_FORMAT_CAPACITIES 0x23
#define SCSI_READ_CAPACITY          0x25
#define SCSI_READ_10                0x28
#define SCSI_WRITE_10               0x2A

#define MIN(a, b) ((a) > (b) ? (b) : (a))

/** Sense key */
typedef enum {
    SENSE_KEY_NO_SENSE          = 0x00,
    SENSE_KEY_RECOVERED_ERROR   = 0x01,
    SENSE_KEY_NOT_READY         = 0x02,
    SENSE_KEY_MEDIUM_ERROR      = 0x03,
    SENSE_KEY_HARDWARE_ERROR    = 0x04,
    SENSE_KEY_ILLEGAL_REQUEST   = 0x05,
    SENSE_KEY_UNIT_ATTENTION    = 0x06,
    SENSE_KEY_DATA_PROTECT      = 0x07,
    SENSE_KEY_BLANK_CHECK       = 0x08,
    SENSE_KEY_VENDOR_SPECIFIC   = 0x09,
    SENSE_KEY_COPY_ABORTED      = 0x0A,
    SENSE_KEY_ABORTED_COMMAND   = 0x0B,
    SENSE_KEY_VOLUME_OVERFLOW   = 0x0D,
    SENSE_KEY_MISCOMPARE        = 0x0E
} scsi_sense_key_t;

/** Additional sense code */
typedef enum {
    ASC_NO_ADDITIONAL_SENSE_INFORMATION = 0x00,
    ASC_PERIPHERAL_DEVICE_WRITE_FAULT   = 0x03,
    ASC_LOGICAL_UNIT_NOT_READY          = 0x04,
    ASC_UNRECOVERED_READ_ERROR          = 0x11,
    ASC_INVALID_COMMAND_OPERATION_CODE  = 0x20,
    ASC_LBA_OUT_OF_RANGE                = 0x21,
    ASC_INVALID_FIELD_IN_CDB            = 0x24,
    ASC_WRITE_PROTECTED                 = 0x27,
    ASC_NOT_READY_TO_READY_CHANGE       = 0x28,
    ASC_FORMAT_ERROR                    = 0x31,
    ASC_MEDIUM_NOT_PRESENT              = 0x3A
} scsi_sense_asc_t;

/** Additional sense code qualifier */
typedef enum {
    ASCQ_NA                             = 0x00,
    ASCQ_FORMAT_COMMAND_FAILED          = 0x01,
    ASCQ_INITIALIZING_COMMAND_REQUIRED  = 0x02,
    ASCQ_OPERATION_IN_PROGRESS          = 0x07
} scsi_sense_ascq_t;

/** Data for REQUEST_SENSE command */
typedef struct {
    scsi_sense_key_t key;        /**< Sense key */
    scsi_sense_asc_t asc;        /**< Additional sense code */
    scsi_sense_ascq_t ascq;      /**< Additional sense code qualifier */
} scsi_sense_info_t;

/** Command Block Wrapper */
typedef struct {
    uint32_t dCBWSignature;             /**< 0x43425355 */
    uint32_t dCBWTag;                   /**< Tag to use in dCSWTag */
    uint32_t dCBWDataTransferLength;    /**< Amount of bytes to transfer */
    uint8_t  bmCBWFlags;                /**< 0x00 Data Out, 0x80 Data IN */
    uint8_t  bCBWLUN;                   /**< LUN to use */
    uint8_t  bCBWCBLength;              /**< Command block valid length */
    uint8_t  CBWCB[16];                 /**< Command block data */
} __attribute__((packed)) msc_cbw_t;

/** Command Status Wrapper */
typedef struct {
    uint32_t dCSWSignature;             /**< 0x53425355 */
    uint32_t dCSWTag;                   /**< Tag from dCBWTag */
    uint32_t dCSWDataResidue;           /**< Difference between processed and required data len */
    uint8_t  bCSWStatus;                /**< Command result */
} __attribute__((packed)) msc_csw_t;

/** SCSI transaction structure */
typedef struct {
    uint8_t cbw_cnt;            /**< Amount of bytes received for CBW packet */
    msc_cbw_t cbw;              /**< The CBW packet data */

    uint32_t bytes_to_read;     /**< Amount of bytes that should be readed */
    uint32_t bytes_to_write;    /**< Amount of bytes that should be written */
    uint32_t bytes_processed;   /**< Amount of bytes already processed */

    uint32_t lba_start;         /**< LBA address to start reading from */
    uint32_t block_count;       /**< Amount of blocks required to read/write */
    uint32_t blocks_processed;  /**< Amount of blocks already processed */

    uint8_t msd_buf[512];       /**< Buffer for reading/writing LBA */

    uint8_t csw_sent;           /**< Amount of bytes sent for CSW packet */
    msc_csw_t csw;              /**< The CSW packet data */
} msc_transaction_t;

/** MSC device data */
typedef struct {
    usbd_device *usbd_dev;
    uint8_t ep_in;
    uint8_t ep_in_size;
    uint8_t ep_out;
    uint8_t ep_out_size;

    const char *vendor_id;
    const char *product_id;
    const char *product_revision_level;
    uint32_t block_count;

    msc_read_block_t read_block;
    msc_write_block_t write_block;

    msc_transaction_t trans;    /**< Currently running transaction state */
    scsi_sense_info_t sense;    /**< Data for REQUEST_SENSE command */
} msc_desc_t;

/** Inquiry command, evpd 0 payload */
static const uint8_t scsi_inquiry_data[] = {
    0x00,   /* Peripheral Qualifier = 0, Peripheral Device Type = 0 */
    0x80,   /* RMB = 1, Reserved = 0 */
    0x04,   /* Version = 0 */
    0x02,   /* Obsolete = 0, NormACA = 0, HiSup = 0, Response Data Format = 2 */
    0x20,   /* Additional Length (n-4) = 31 + 4 */
    0x00,   /* SCCS = 0, ACC = 0, TPGS = 0, 3PC = 0, Reserved = 0, Protect = 0 */
    0x00,   /* BQue = 0, EncServ = 0, VS = 0, MultiP = 0, MChngr = 0, Obsolete = 0, Addr16 = 0 */
    0x00,   /* Obsolete = 0, Wbus16 = 0, Sync = 0, Linked = 0, CmdQue = 0, VS = 0 */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /* Vendor identification */
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, /* Product identification */
    0x20, 0x20, 0x20, 0x20 /* Product Revision Level */
};

/** Inquiry command evpd 1 payload */
static const uint8_t scsi_inquiry_sn_data[] = {
    0x00,
    0x80,
    0x00,
    0x06, /* Amount of characters below */
    '1','2','3','4','5','6'
};

/** Request sense command default payload */
static const uint8_t scsi_request_sense_data[] = {
    0x70,   /* VALID = 0, Response Code = 112 */
    0x00,   /* Obsolete = 0 */
    0x00,   /* Filemark = 0, EOM = 0, ILI = 0, Reserved = 0, Sense Key = 0 */
    0, 0, 0, 0, /* Information = 0 */
    0x0a,   /* Additional Sense Length = 10 */
    0, 0, 0, 0, /* Command Specific Info = 0 */
    0x00,   /* Additional Sense Code (ASC) = 0 */
    0x00,   /* Additional Sense Code Qualifier (ASCQ) = 0 */
    0x00,   /* Field Replaceable Unit Code (FRUC) = 0 */
    0x00,   /* SKSV = 0, SenseKeySpecific[0] = 0 */
    0x00,   /* SenseKeySpecific[0] = 0 */
    0x00    /* SenseKeySpecific[0] = 0 */
};

/** The global MSC descriptor (only on device supported) */
static msc_desc_t msci_desc;

/**
 * Set data for REQUEST_SENSE command
 *
 * @param ms            MSC device descriptor
 * @param key           Sense key
 * @param asc           Additional sense code
 * @param ascq          Additional sense code qualifier
 */
static void scsi_set_status(msc_desc_t *ms, scsi_sense_key_t key,
        scsi_sense_asc_t asc, scsi_sense_ascq_t ascq)
{
    ms->sense.key = key;
    ms->sense.asc = asc;
    ms->sense.ascq = ascq;

    /* Sending error, ignore the command payload */
    if (key != SENSE_KEY_NO_SENSE) {
        ms->trans.bytes_to_write = 0;
        ms->trans.bytes_to_read = 0;
        ms->trans.csw.bCSWStatus = CSW_STATUS_FAILED;
    }
}

/**
 * Set status good for REQUEST_SENSE command
 *
 * @param ms         MSC device descriptor
 */
static void scsi_set_status_good(msc_desc_t *ms)
{
    scsi_set_status(ms, SENSE_KEY_NO_SENSE, ASC_NO_ADDITIONAL_SENSE_INFORMATION,
           ASCQ_NA);
}

static void scsi_finish_transaction(msc_transaction_t *trans)
{
        trans->lba_start = 0xffffffff;
        trans->block_count = 0;
        trans->blocks_processed = 0;
        trans->cbw_cnt = 0;
        trans->bytes_to_read = 0;
        trans->bytes_to_write = 0;
        trans->bytes_processed = 0;
        trans->csw_sent = 0;
}

static void scsi_verify_rw_range(msc_desc_t *ms)
{
    if (ms->trans.lba_start + ms->trans.block_count - 1 > ms->block_count) {
        scsi_set_status(ms, SENSE_KEY_ILLEGAL_REQUEST, ASC_LBA_OUT_OF_RANGE,
                ASCQ_NA);
    } else {
        scsi_set_status_good(ms);
    }
}

static void scsi_read_6(msc_desc_t *ms)
{
    uint8_t *buf;

    buf = ms->trans.cbw.CBWCB;
    ms->trans.lba_start = (buf[2] << 8) | buf[3];
    ms->trans.block_count = buf[4];
    ms->trans.blocks_processed = 0;
    /* blocks * block_size (512) */
    ms->trans.bytes_to_write = ms->trans.block_count << 9;
    scsi_verify_rw_range(ms);
}

static void scsi_write_6(msc_desc_t *ms)
{
    uint8_t *buf;

    buf = ms->trans.cbw.CBWCB;

    ms->trans.lba_start = ((0x1f & buf[1]) << 16) | (buf[2] << 8) | buf[3];
    ms->trans.block_count = buf[4];
    ms->trans.blocks_processed = 0;
    ms->trans.bytes_to_read = ms->trans.block_count << 9;
    scsi_verify_rw_range(ms);
}

static void scsi_write_10(msc_desc_t *ms)
{
    uint8_t *buf;

    buf = ms->trans.cbw.CBWCB;
    ms->trans.lba_start = (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) |
                           buf[5];
    ms->trans.block_count = (buf[7] << 8) | buf[8];
    ms->trans.blocks_processed = 0;
    ms->trans.bytes_to_read = ms->trans.block_count << 9;
    scsi_verify_rw_range(ms);
}

static void scsi_read_10(msc_desc_t *ms)
{
    uint8_t *buf;

    buf = ms->trans.cbw.CBWCB;
    ms->trans.lba_start = (buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) |
                           buf[5];
    ms->trans.block_count = (buf[7] << 8) | buf[8];
    ms->trans.bytes_to_write = ms->trans.block_count << 9;
    scsi_verify_rw_range(ms);
}

static void scsi_read_capacity(msc_desc_t *ms)
{
    ms->trans.msd_buf[0] = ms->block_count >> 24;
    ms->trans.msd_buf[1] = 0xff & (ms->block_count >> 16);
    ms->trans.msd_buf[2] = 0xff & (ms->block_count >> 8);
    ms->trans.msd_buf[3] = 0xff & ms->block_count;

    /* Block size: 512 */
    ms->trans.msd_buf[4] = 0;
    ms->trans.msd_buf[5] = 0;
    ms->trans.msd_buf[6] = 2;
    ms->trans.msd_buf[7] = 0;
    ms->trans.bytes_to_write = 8;
    scsi_set_status_good(ms);
}

static void scsi_format_unit(msc_desc_t *ms)
{
    memset(ms->trans.msd_buf, 0, 512);
    for (uint32_t i = 0; i < ms->block_count; i++) {
        (*ms->write_block)(i, ms->trans.msd_buf);
    }
    scsi_set_status_good(ms);
}

static void scsi_request_sense(msc_desc_t *ms)
{
    uint8_t *buf;

    buf = ms->trans.cbw.CBWCB;

    ms->trans.bytes_to_write = buf[4]; /* allocation length */
    memcpy(ms->trans.msd_buf, scsi_request_sense_data,
           sizeof(scsi_request_sense_data));

    ms->trans.msd_buf[2] = ms->sense.key;
    ms->trans.msd_buf[12] = ms->sense.asc;
    ms->trans.msd_buf[13] = ms->sense.ascq;
}

static void scsi_mode_sense_6(msc_desc_t *ms)
{
    ms->trans.bytes_to_write = 4;

    ms->trans.msd_buf[0] = 3;  /* Num bytes that follow */
    ms->trans.msd_buf[1] = 0;  /* Medium Type */
    ms->trans.msd_buf[2] = 0;  /* Device specific param */
    ms->trans.csw.dCSWDataResidue = 4;
}

static void scsi_inquiry(msc_desc_t *ms)
{
    uint8_t evpd;
    uint8_t *buf;

    buf = ms->trans.cbw.CBWCB;
    evpd = 1 & buf[1];

    if (0 == evpd) {
        size_t len;
        ms->trans.bytes_to_write = sizeof(scsi_inquiry_data);
        memcpy(ms->trans.msd_buf, scsi_inquiry_data,
               sizeof(scsi_inquiry_data));

        len = MIN(strlen(ms->vendor_id), 8);
        memcpy(&ms->trans.msd_buf[8], ms->vendor_id, len);

        len = MIN(strlen(ms->product_id), 16);
        memcpy(&ms->trans.msd_buf[16], ms->product_id, len);

        len = MIN(strlen(ms->product_revision_level), 4);
        memcpy(&ms->trans.msd_buf[32], ms->product_revision_level, len);

        ms->trans.csw.dCSWDataResidue = sizeof(scsi_inquiry_data);
        scsi_set_status_good(ms);
    } else if (evpd == 1) {
        ms->trans.bytes_to_write = sizeof(scsi_inquiry_sn_data);
        memcpy(ms->trans.msd_buf, scsi_inquiry_sn_data,
                sizeof(scsi_inquiry_sn_data));
        ms->trans.csw.dCSWDataResidue = sizeof(scsi_inquiry_sn_data);
        scsi_set_status_good(ms);
    } else {
        /* TODO: Add VPD 0x83 support */
        /* TODO: Add VPD 0x00 support */
    }
}

static void scsi_read_format_capacities(msc_desc_t *ms)
{
    ms->trans.msd_buf[3] = 0x08;
    ms->trans.msd_buf[4] = ms->block_count >> 24;
    ms->trans.msd_buf[5] = 0xff & (ms->block_count >> 16);
    ms->trans.msd_buf[6] = 0xff & (ms->block_count >> 8);
    ms->trans.msd_buf[7] = 0xff & ms->block_count;

    ms->trans.msd_buf[8] = 0x02;
    ms->trans.msd_buf[9] = 0x00;
    ms->trans.msd_buf[10] = 0x02;
    ms->trans.msd_buf[11] = 0x00;
    ms->trans.bytes_to_write = 12;
    scsi_set_status_good(ms);
}

static void scsi_command(msc_desc_t *ms)
{
    /* Setup the default success */
    ms->trans.csw_sent = 0;
    ms->trans.csw.dCSWSignature = CSW_SIGNATURE;
    ms->trans.csw.dCSWTag = ms->trans.cbw.dCBWTag;
    ms->trans.csw.dCSWDataResidue = 0;
    ms->trans.csw.bCSWStatus = CSW_STATUS_SUCCESS;

    ms->trans.bytes_to_write = 0;
    ms->trans.bytes_to_read = 0;
    ms->trans.bytes_processed = 0;
    ms->trans.blocks_processed = 0;

    switch (ms->trans.cbw.CBWCB[0]) {
        case SCSI_TEST_UNIT_READY:
        case SCSI_SEND_DIAGNOSTIC:
            /* Do nothing, just send the success. */
            scsi_set_status_good(ms);
            break;
        case SCSI_FORMAT_UNIT:
            scsi_format_unit(ms);
            break;
        case SCSI_REQUEST_SENSE:
            scsi_request_sense(ms);
            break;
        case SCSI_MODE_SENSE_6:
            scsi_mode_sense_6(ms);
            break;
        case SCSI_READ_6:
            scsi_read_6(ms);
            break;
        case SCSI_INQUIRY:
            scsi_inquiry(ms);
            break;
        case SCSI_READ_CAPACITY:
            scsi_read_capacity(ms);
            break;
        case SCSI_READ_10:
            scsi_read_10(ms);
            break;
        case SCSI_WRITE_6:
            scsi_write_6(ms);
            break;
        case SCSI_WRITE_10:
            scsi_write_10(ms);
            break;
        case SCSI_READ_FORMAT_CAPACITIES:
            scsi_read_format_capacities(ms);
            break;
        case SCSI_PREVENT_ALLOW_MEDIUM_REMOVAL:
            scsi_set_status_good(ms);
            break;
        default:
            scsi_set_status(ms, SENSE_KEY_ILLEGAL_REQUEST,
                        ASC_INVALID_COMMAND_OPERATION_CODE,
                        ASCQ_NA);
            break;
    }
}

/**
 * Handle the USB OUT request
 *
 * @param usbd_dev      The USB device that received the request
 * @param ep            Endpoint number
 */
static void msci_data_rx(usbd_device *usbd_dev, uint8_t ep)
{
    uint32_t lba, left;
    msc_desc_t *ms = &msci_desc;
    msc_transaction_t *trans = &ms->trans;

    /* CBW reception */
    left = MIN(sizeof(trans->cbw) - trans->cbw_cnt, ms->ep_in_size);
    if (left > 0) {
        trans->cbw_cnt += usbd_ep_read_packet(usbd_dev, ep,
                &((uint8_t *) &trans->cbw)[trans->cbw_cnt], left);

        if (sizeof(trans->cbw) == trans->cbw_cnt) {
            scsi_command(ms);
            if (trans->bytes_to_read > 0) {
                /* we are receiving, more data in next packet */
                return;
            }
        }
    }

    /* data reading */
    if (trans->bytes_processed < trans->bytes_to_read) {
        left = MIN(trans->bytes_to_read - trans->bytes_processed, ms->ep_in_size);
        trans->bytes_processed += usbd_ep_read_packet(usbd_dev, ep,
                &trans->msd_buf[0x1ff & trans->bytes_processed], left);

        if (trans->block_count > 0 && (trans->bytes_processed & 0x1ff) == 0) {
                lba = trans->lba_start + trans->blocks_processed;
                (*ms->write_block)(lba, trans->msd_buf);
                trans->blocks_processed++;
        }

        left = MIN(sizeof(trans->csw) - trans->csw_sent, ms->ep_out_size);
        if (left > 0) {
            trans->csw_sent += usbd_ep_write_packet(usbd_dev, ms->ep_in,
                    &((uint8_t *) &trans->csw)[trans->csw_sent], left);
        }
    /* data writing, fill tx buffer here, rest will be done in tx function */
    } else if (trans->bytes_processed < trans->bytes_to_write) {
        if (trans->block_count > 0 && (trans->bytes_processed && 0x1ff) == 0) {
            lba = trans->lba_start + trans->blocks_processed;
            (*ms->read_block)(lba, trans->msd_buf);
            trans->blocks_processed++;
        }

        left = MIN(trans->bytes_to_write - trans->bytes_processed, ms->ep_out_size);
        trans->bytes_processed += usbd_ep_write_packet(usbd_dev, ms->ep_in,
                &trans->msd_buf[0x1ff & trans->bytes_processed], left);
    /* everything written/readed or nothing to read/write */
    } else {
        if (trans->block_count > 0 && trans->blocks_processed == trans->block_count) {
                lba = trans->lba_start + trans->blocks_processed;
                (*ms->write_block)(lba, trans->msd_buf);
                trans->blocks_processed = 0;
        }

        left = MIN(sizeof(trans->csw) - trans->csw_sent, ms->ep_out_size);
        if (left > 0) {
            trans->csw_sent += usbd_ep_write_packet(usbd_dev, ms->ep_in,
                    &((uint8_t *) &trans->csw)[trans->csw_sent], left);
        } else if (sizeof(trans->csw) == trans->csw_sent) {
            scsi_finish_transaction(trans);
        }
    }
}

/** @brief Handle the USB 'IN' requests. */
/**
 * Handle the USB IN request
 *
 * @param usbd_dev      The USB device that received the request
 * @param ep            Endpoint number
 */
static void msci_data_tx(usbd_device *usbd_dev, uint8_t ep)
{
    uint32_t lba, left;
    msc_desc_t *ms = &msci_desc;
    msc_transaction_t *trans = &ms->trans;

    /* have some bytes to send */
    if (trans->bytes_processed < trans->bytes_to_write) {
        if (trans->block_count != 0 && (0x1ff & trans->bytes_processed) == 0) {
            lba = trans->lba_start + trans->blocks_processed;
            ms->read_block(lba, trans->msd_buf);
            trans->blocks_processed++;
        }
        left = MIN(trans->bytes_to_write - trans->bytes_processed,
                ms->ep_out_size);
        trans->bytes_processed += usbd_ep_write_packet(usbd_dev, ep,
                &trans->msd_buf[0x1ff & trans->bytes_processed], left);
        return;
    }

    left = MIN(sizeof(trans->csw) - trans->csw_sent, ms->ep_out_size);
    /* send CSW */
    if (left > 0) {
        trans->csw_sent += usbd_ep_write_packet(usbd_dev, ep,
                &((uint8_t *) &trans->csw)[trans->csw_sent], left);
    } else if (sizeof(trans->csw) == trans->csw_sent) {
        scsi_finish_transaction(trans);
    }
}

/** @brief Handle various control requests related to the msc storage
 *     interface.
 */
/**
 * Handle the control requests on the MSC interface
 *
 * @param usbd_dev      The USB device that received the request
 * @param req           The USB request
 * @param [out] buf     Buffer to store result
 * @param [out] len     Lenght of the stored resultk
 * @param complete      The complete callback
 */
static enum usbd_request_return_codes msci_control_request(
        usbd_device *usbd_dev, struct usb_setup_data *req,
        uint8_t **buf, uint16_t *len, usbd_control_complete_callback *complete)
{
    (void)complete;
    (void)usbd_dev;

    switch (req->bRequest) {
        case USB_MSC_REQ_BULK_ONLY_RESET:
            /* Do any special reset code here. */
            scsi_finish_transaction(&msci_desc.trans);
            return USBD_REQ_HANDLED;
        case USB_MSC_REQ_GET_MAX_LUN:
            /* We have only one LUN */
            *buf[0] = 0;
            *len = 1;
            return USBD_REQ_HANDLED;
    }

    return USBD_REQ_NEXT_CALLBACK;
}

/**
 * Process the set config call from the USB stack
 *
 * @param usbd_dev      The USB device that received the request
 * @param wValue        The wValue of the config request
 */
static void msci_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
    msc_desc_t *desc = &msci_desc;
    (void)wValue;

    usbd_ep_setup(usbd_dev, desc->ep_in, USB_ENDPOINT_ATTR_BULK,
              desc->ep_in_size, msci_data_tx);
    usbd_ep_setup(usbd_dev, desc->ep_out, USB_ENDPOINT_ATTR_BULK,
              desc->ep_out_size, msci_data_rx);

    usbd_register_control_callback(
                usbd_dev,
                USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                msci_control_request);
}

/**
 * Initialize the USB Mass Storage
 *
 * @param usbd_dev      The USB device used
 * @param ep_in         Number of the USB IN endpoint
 * @param ep_in_size    The maximum endpoint size (8 to 64)
 * @param ep_out        Number of the USB OUT endpoint
 * @param ep_out_size   The maximum endpoint size (8 to 64)
 * @param vendor_id     The SCSI vendor ID (up to 8 characters)
 * @param product_id    The SCSI product ID (up to 16 characters)
 * @param product_revision_level    The SCSI revision (up to 4 characters)
 * @param read_block    Function to call when host request read of a LBA block
 * @param write_block   Function to call when host request write to a LBA block
 * @param block_count   Amount of 512B blocks available
 */
void Msc_Init(usbd_device *usbd_dev, uint8_t ep_in, uint8_t ep_in_size,
        uint8_t ep_out, uint8_t ep_out_size, const char *vendor_id,
        const char *product_id, const char *product_revision_level,
        msc_read_block_t read_block, msc_write_block_t write_block,
        uint32_t block_count)
{
    msci_desc.usbd_dev = usbd_dev;
    msci_desc.ep_in = ep_in;
    msci_desc.ep_in_size = ep_in_size;
    msci_desc.ep_out = ep_out;
    msci_desc.ep_out_size = ep_out_size;
    msci_desc.vendor_id = vendor_id;
    msci_desc.product_id = product_id;
    msci_desc.product_revision_level = product_revision_level;
    msci_desc.read_block = read_block;
    msci_desc.write_block = write_block;
    msci_desc.block_count = block_count;

    memset((uint8_t *)&msci_desc.trans, 0x00, sizeof(msci_desc.trans));
    msci_desc.trans.lba_start = 0xffffffff;
    scsi_set_status_good(&msci_desc);

    usbd_register_set_config_callback(usbd_dev, msci_set_config);
}
