/**
 * @file    drivers/rfm69.c
 * @brief   HopeRF RFM69 wireless transceiver driver, same as SX1231
 *
 * https://cdn.sparkfun.com/datasheets/Wireless/General/RFM69HCW-V1.1.pdf
 */

#ifndef __DRIVERS_RFM69_H
#define __DRIVERS_RFM69_H

#include <types.h>

/** Device descriptor */
typedef struct {
    uint8_t spi_device;  /**< SPI device the RFM is connected to */
    uint32_t cs_port;    /**< MCU port the CS signal is connected to */
    uint8_t cs_pad;      /**< MCU pin the CS signal is connected to */
    uint32_t reset_port; /**< MCU port the reset signal is connected to */
    uint8_t reset_pad;   /**< MCU pin the reset signal is connected to */
    uint32_t io0_port;   /**< MCU port the IO0 signal is connected to */
    uint32_t io0_pad;    /**< MCU pin the IO0 signal is connected to */
    uint32_t io4_port;   /**< MCU port the IO4 signal is connected to */
    uint32_t io4_pad;    /**< MCU pin the IO4 signal is connected to */
    bool is_hxx;         /**< True if RFM69HW or HCW is used */
    bool high_power;     /**< If true, the >17dBm output is used */
    uint8_t payload_len; /**< Fixed payload length if used */
} rfm69_desc_t;

typedef enum {
    RFM69_ENC_NONE = 0,       /**< No DC-free encoding */
    RFM69_ENC_MANCHESTER = 1, /**< Manchester DC-free encoding */
    RFM69_ENC_WHITENING = 2,  /**< Whitening DC-free encoding */
} rfm69_encoding_t;

typedef enum {
    RFM69_FSK_NO_SHAPING = 0,   /**< Frequency shift keying modulator, no modulation shaping */
    RFM69_FSK_BT_1_0 = 0x1,     /**< Frequency shift keying modulator, Gaussian filter BT=1.0 */
    RFM69_FSK_BT_0_5 = 0x2,     /**< Frequency shift keying modulator, Gaussian filter BT=0.5 */
    RFM69_FSK_BT_0_3 = 0x3,     /**< Frequency shift keying modulator, Gaussian filter BT=0.3 */
    RFM69_OOK_NO_SHAPING = 0x8, /**< On-Off keying modulator, no modulation shaping */
    RFM69_OOK_BR = 0x9,         /**< On-Off keying modulator, filtering with fcutoff=BR */
    RFM69_OOK_2BR = 0xa,        /**< On-Off keying modulator, filtering with fcutoff=2*BR */
} rfm69_modulation_t;

/** RX filter bandwidth in kHz */
typedef enum {
    RFM69_BW_FSK_2_6K = 0x17,
    RFM69_BW_FSK_3_1K = 0x0F,
    RFM69_BW_FSK_3_9K = 0x07,
    RFM69_BW_FSK_5_2K = 0x16,
    RFM69_BW_FSK_6_3K = 0x0E,
    RFM69_BW_FSK_7_8K = 0x06,
    RFM69_BW_FSK_10_4K = 0x15,
    RFM69_BW_FSK_12_5K = 0x0D,
    RFM69_BW_FSK_15_6K = 0x05,
    RFM69_BW_FSK_20_8K = 0x14,
    RFM69_BW_FSK_25K = 0x0C,
    RFM69_BW_FSK_31_3K = 0x04,
    RFM69_BW_FSK_41_7K = 0x13,
    RFM69_BW_FSK_50K = 0x0B,
    RFM69_BW_FSK_62_5K = 0x03,
    RFM69_BW_FSK_83_3K = 0x12,
    RFM69_BW_FSK_100K = 0x0A,
    RFM69_BW_FSK_125K = 0x02,
    RFM69_BW_FSK_166_7K = 0x11,
    RFM69_BW_FSK_200K = 0x09,
    RFM69_BW_FSK_250K = 0x01,
    RFM69_BW_FSK_333_3K = 0x10,
    RFM69_BW_FSK_400K = 0x08,
    RFM69_BW_FSK_500K = 0x00,
    RFM69_BW_OOK_1_3K = 0x17,
    RFM69_BW_OOK_1_6K = 0x0F,
    RFM69_BW_OOK_2K = 0x07,
    RFM69_BW_OOK_2_6K = 0x16,
    RFM69_BW_OOK_3_1K = 0x0E,
    RFM69_BW_OOK_3_9K = 0x06,
    RFM69_BW_OOK_5_2K = 0x15,
    RFM69_BW_OOK_6_3K = 0x0D,
    RFM69_BW_OOK_7_8K = 0x05,
    RFM69_BW_OOK_10_4K = 0x14,
    RFM69_BW_OOK_12_5K = 0x0C,
    RFM69_BW_OOK_15_6K = 0x04,
    RFM69_BW_OOK_20_8K = 0x13,
    RFM69_BW_OOK_25K = 0x0B,
    RFM69_BW_OOK_31_3K = 0x03,
    RFM69_BW_OOK_41_7K = 0x12,
    RFM69_BW_OOK_50K = 0x0A,
    RFM69_BW_OOK_62_5K = 0x02,
    RFM69_BW_OOK_83_3K = 0x11,
    RFM69_BW_OOK_100K = 0x09,
    RFM69_BW_OOK_125K = 0x01,
    RFM69_BW_OOK_166_7K = 0x10,
    RFM69_BW_OOK_200K = 0x08,
    RFM69_BW_OOK_250K = 0x00,
} rfm69_rx_bandwidth_t;

typedef struct {
    uint32_t frequency_hz;         /**< Carrier frequency in Hz */
    uint32_t bitrate_bps;          /**< Bitrate in bits per second, or chip rate for menchester */
    uint32_t freq_deviation_hz;    /**< Frequency deviation in Hz */
    rfm69_modulation_t modulation; /**< Signal modulation */
    rfm69_rx_bandwidth_t rx_bw;    /**< Input filter bandwidth */
} rfm69_config_t;

// clang-format off
/**
 * Frame format definition
 *
 * Fixed length
 * | ----------------------Header---------------------|----------------------------Payload ---------------------| Suffix                  |
 * | Preamble bytes (0-65545) | Sync word (0-8 bytes) | Address byte (optional) | Message (0-255, AES optional) | CRC (optional, 2 bytes) |
 *
 * Variable length
 * | ----------------------Header---------------------|-------------------------------Payload ---------------------------------| Suffix                  |
 * | Preamble bytes (0-65545) | Sync word (0-8 bytes) | Length bytes | Address byte (optional) | Message (0-255, AES optional) | CRC (optional, 2 bytes) |
 *
 * The FIFO is 66 bytes long, the payload length must be equal or lower
 */
// clang-format on

typedef struct {
    bool variable_len;   /**< True for variable length frames, false for fixed */
    uint8_t payload_len; /**< Maximum payload length for variable frames, or len for fixed frame */
    uint16_t preamble_len; /**< Size of preamble to be sent */

    struct {
        uint8_t len;       /**< Length of sync word in bytes, 1-8 bytes,  0 to disable */
        uint8_t value[8];  /**< Sync word bytes, 0x00 not allowed */
        uint8_t tolerance; /**< Number of tolerated errors in bytes, 0 to 7 */
    } sync;

    bool crc; /**< Enable CRC generation/checking, the CRC is stripped from payload */
    rfm69_encoding_t encoding; /**< DC free encoding method */

    struct {
        bool enable;     /** Enable frame filtering based on address (first byte of payload), add to
                            payload manually at TX */
        uint8_t unicast; /** Unicast address to listen for */
        uint8_t
            broadcast; /** Broadcast address to listen for, set to same as unicast if not needed */
    } filter;

    struct {
        bool enable;     /**< Enable AES encryption, payload limited to 66 bytes */
        uint8_t key[16]; /**< 16 bytes of the AES encryption key */
    } aes;
} rfm69_frame_t;

/**
 * Set transmit power
 *
 * @note the HW version supports -2 to 20 dBm, the W version -18 to 13 dBm

 * @param desc      Device descriptor
 * @param dBm       Desired power in dBm (-18 to 20), 20 dBm is limited to 1 % duty cycle and
 antenna must be matched
 */
void RFM69_SetPowerDBm(rfm69_desc_t *desc, int8_t dBm);

/**
 * Set communication frame format
 *
 * @param desc      Device descriptor
 * @param format    Required frame format
 */
void RFM69_SetFrameFormat(rfm69_desc_t *desc, const rfm69_frame_t *format);

/**
 * Set center frequency
 *
 * @param desc      Device descriptor
 * @param freq_hz   Required frequency in Hz
 */
void RFM69_SetFrequencyHz(const rfm69_desc_t *desc, uint32_t freq_hz);

/**
 * Set radio parameters
 *
 * @param desc      Device descriptor
 * @param config    Radio configuration parameters
 */
void RFM69_SetRadioConfig(const rfm69_desc_t *desc, const rfm69_config_t *config);

/**
 * Measure RSSI to check if nothing else is transmitting
 *
 * @param desc      Device descriptor
 * @param threshold Threshold in dBm for empty detection (e.g. -100 dBm)
 * @return  true if RSSI is low (nothing transmitting)
 */
bool RFM69_IsChannelEmpty(const rfm69_desc_t *desc, int8_t threshold);

/**
 * Send frame
 *
 * @param desc      Device descriptor
 * @param data      Data buffer
 * @param len       Length of the data to be sent
 */
void RFM69_Send(const rfm69_desc_t *desc, const uint8_t *data, uint8_t len);

/**
 * Receive frame
 *
 * @note The RX is automatically restarted after reception
 *
 * @param desc      Device descriptor
 * @param data      Data buffer
 * @param len       Length of the data to be received
 * @param rssi      Packet RSSI stored here if not NULL
 *
 * @return Amount of bytes received or 0 if nothing received yet
 */
uint8_t RFM69_Receive(const rfm69_desc_t *desc, uint8_t *data, uint8_t max_len, int8_t *rssi);

/**
 * Start reception
 *
 * @note Needs to be called after any configuration change, sending data, reading frame,...
 *
 * @param desc      Device descriptor
 */
void RFM69_StartReceiver(const rfm69_desc_t *desc);

/**
 * Initialize the RFM69 module
 *
 * @note SPI clock must be 10 MHz or lower
 * @note DIO0 in RX mode signals PayloadReady
 *
 * @param desc          Device descriptor
 * @param spi_device    SPI device ID
 * @param cd_port       Port of chip select
 * @param cd_pad        Pad of chip select
 * @param reset_port    Port of reset pin or 0xFF if not available
 * @param reset_pad     Pad of reset pin
 * @param is_hxx        True if device is High power version
 */
bool RFM69_Init(rfm69_desc_t *desc, uint8_t spi_device, uint32_t cs_port, uint8_t cs_pad,
    uint32_t reset_port, uint8_t reset_pad, bool is_hxx);

#endif
