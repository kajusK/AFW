/**
 * @file    drivers/rfm95.h
 * @brief   Driver for Hope RF RFM95 and similar LORA RF modules
 */

#ifndef __DRIVERS_RFM95_H
#define __DRIVERS_RFM95_H

#include <types.h>

/* Signal bandwidth */
typedef enum {
    RFM95_BW_125k,
    RFM95_BW_250k,
    RFM95_BW_500k,
} rfm95_bw_t;

/** Spreading factor */
typedef enum {
    RFM95_SF_7 = 7,
    RFM95_SF_8 = 8,
    RFM95_SF_9 = 9,
    RFM95_SF_10 = 10,
    RFM95_SF_11 = 11,
    RFM95_SF_12 = 12,
} rfm95_sf_t;

/** Lora regions (frequency to use) */
typedef enum {
    RFM95_REGION_AU915,
    RFM95_REGION_EU863,
    RFM95_REGION_US902,
    RFM95_REGION_AS920
} rfm95_lora_region_t;

/** The RFM device descriptor */
typedef struct {
    uint8_t spi_device;     /**< SPI device the RFM is connected to */
    uint32_t cs_port;       /**< MCU port the CS signal is connected to */
    uint8_t cs_pad;         /**< MCU pin the CS signal is connected to */
    uint32_t reset_port;    /**< MCU port the reset signal is connected to */
    uint8_t reset_pad;      /**< MCU pin the reset signal is connected to */
    uint32_t io0_port;      /**< MCU port the IO0 signal is connected to */
    uint32_t io0_pad;       /**< MCU pin the IO0 signal is connected to */
    const uint8_t (*region)[3];
} rfm95_desc_t;

/**
 * Set transmit power
 *
 * Output range is from -4 to 14dBm, W versions can use up to 20 dBm. Over
 * 17 dBm, continuous operation is not possible. Up to 1% duty cycle and antenna
 * with VSWR up to 3:1 should be used.
 *
 * As the RFM95 module doesn't seem to have RFO output connected, PA must
 * be enabled all the time, therefore the usable range starts really at 2 dBm
 *
 * @param desc      The RFM device descriptor
 * @param power     Required power from 2 to 20 dBm
 */
extern void RFM95_SetPowerDBm(const rfm95_desc_t *desc, int8_t power);

/**
 * Set Lora transmission bandwidth and spreading factor
 *
 * Use https://www.thethingsnetwork.org/airtime-calculator to calculate
 * required airtime (and also check max amount of bytes and possible datarates
 * for each region).
 *
 * EU supports only 125 and 250 kHz (only SF7 supported here).
 * Higher SF -> lower baudrate -> higher sensitivity -> longer airtime
 * Higher BW -> higher baudrate -> lower sensitivity -> shorter airtime
 *
 * @param desc          The RFM device descriptor
 * @param bandwidth     Bandwidth selection
 * @param sf            Spreading factor
 */
extern void RFM95_SetLoraParams(const rfm95_desc_t *desc, rfm95_bw_t bandwidth,
        rfm95_sf_t sf);

/**
 * Configure region we are in (sets frequency range)
 *
 * @param desc      The RFM device descriptor
 * @param region        Region settings to be used
 */
extern void RFM95_SetLoraRegion(rfm95_desc_t *desc, rfm95_lora_region_t region);

/**
 * Send raw Lora modulated data
 *
 * @param desc      The RFM device descriptor
 * @param data      Data to be sent
 * @param len       Length of the data buffer
 */
extern void RFM95_LoraSend(const rfm95_desc_t *desc, const uint8_t *data,
        size_t len);

/**
 * Power off the device
 *
 * Call RFM95_LoraInit to power it on again
 *
 * @param desc          The RFM device descriptor
 */
extern void RFM95_PowerOff(rfm95_desc_t *desc);

/**
 * Power on and initialize the RFM in LoRa mode
 *
 * @param desc          The RFM device descriptor
 */
extern void RFM95_LoraInit(rfm95_desc_t *desc);

/**
 * Initialize RFM module
 *
 * The module is in power off state after initialization, call LoraInit to
 * power on.
 *
 * @param desc          The RFM device descriptor
 * @param spi_device    The SPI device to use
 * @param cs_port       Port of the CS pin
 * @param cs_pad        Pin of the CS pin
 * @param reset_port    Port of the RESET pin
 * @param reset_pad     Pin of the RESET pin
 * @param io0_port      Port of the IO0 pin
 * @param io0_pad       Pin of the IO0 pin
 *
 * @return True if module is present, false if not responding
 */
extern bool RFM95_Init(rfm95_desc_t *desc, uint8_t spi_device, uint32_t cs_port,
        uint8_t cs_pad, uint32_t reset_port, uint8_t reset_pad,
        uint32_t io0_port, uint8_t io0_pad);

#endif
