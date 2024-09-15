/*
 * Copyright (C) 2024 Jakub Kaderka
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
 * @file    drivers/rfm69.c
 * @brief   HopeRF RFM69 wireless transceiver driver, same as SX1231
 *
 * https://cdn.sparkfun.com/datasheets/Wireless/General/RFM69HCW-V1.1.pdf
 *
 * @addtogroup drivers
 * @{
 */

#include "hal/io.h"
#include "hal/spi.h"
#include "utils/time.h"
#include "rfm69.h"

/* Oscillator frequency */
#define FXOSC (32000000UL)

/* Timeout for pooling operations */
#define CMD_TIMEOUT_MS 50

/* Registers */
#define REG_FIFO          0x00
#define REG_OPMODE        0x01
#define REG_DATAMODUL     0x02
#define REG_BITRATEMSB    0x03
#define REG_BITRATELSB    0x04
#define REG_FDEVMSB       0x05
#define REG_FDEVLSB       0x06
#define REG_FRFMSB        0x07
#define REG_FRFMID        0x08
#define REG_FRFLSB        0x09
#define REG_OSC1          0x0A
#define REG_AFCCTRL       0x0B
#define REG_LOWBAT        0x0C
#define REG_LISTEN1       0x0D
#define REG_LISTEN2       0x0E
#define REG_LISTEN3       0x0F
#define REG_VERSION       0x10
#define REG_PALEVEL       0x11
#define REG_PARAMP        0x12
#define REG_OCP           0x13
#define REG_LNA           0x18
#define REG_RXBW          0x19
#define REG_AFCBW         0x1A
#define REG_OOKPEAK       0x1B
#define REG_OOKAVG        0x1C
#define REG_OOKFIX        0x1D
#define REG_AFCFEI        0x1E
#define REG_AFCMSB        0x1F
#define REG_AFCLSB        0x20
#define REG_FEIMSB        0x21
#define REG_FEILSB        0x22
#define REG_RSSICONFIG    0x23
#define REG_RSSIVALUE     0x24
#define REG_DIOMAPPING1   0x25
#define REG_DIOMAPPING2   0x26
#define REG_IRQFLAGS1     0x27
#define REG_IRQFLAGS2     0x28
#define REG_RSSITHRESH    0x29
#define REG_RXTIMEOUT1    0x2A
#define REG_RXTIMEOUT2    0x2B
#define REG_PREAMBLEMSB   0x2C
#define REG_PREAMBLELSB   0x2D
#define REG_SYNCCONFIG    0x2E
#define REG_SYNCVALUE1    0x2F
#define REG_SYNCVALUE2    0x30
#define REG_SYNCVALUE3    0x31
#define REG_SYNCVALUE4    0x32
#define REG_SYNCVALUE5    0x33
#define REG_SYNCVALUE6    0x34
#define REG_SYNCVALUE7    0x35
#define REG_SYNCVALUE8    0x36
#define REG_PACKETCONFIG1 0x37
#define REG_PAYLOADLENGTH 0x38
#define REG_NODEADRS      0x39
#define REG_BROADCASTADRS 0x3A
#define REG_AUTOMODES     0x3B
#define REG_FIFOTHRESH    0x3C
#define REG_PACKETCONFIG2 0x3D
#define REG_AESKEY1       0x3E
#define REG_AESKEY2       0x3F
#define REG_AESKEY3       0x40
#define REG_AESKEY4       0x41
#define REG_AESKEY5       0x42
#define REG_AESKEY6       0x43
#define REG_AESKEY7       0x44
#define REG_AESKEY8       0x45
#define REG_AESKEY9       0x46
#define REG_AESKEY10      0x47
#define REG_AESKEY11      0x48
#define REG_AESKEY12      0x49
#define REG_AESKEY13      0x4A
#define REG_AESKEY14      0x4B
#define REG_AESKEY15      0x4C
#define REG_AESKEY16      0x4D
#define REG_TEMP1         0x4E
#define REG_TEMP2         0x4F
#define REG_TESTLNA       0x58
#define REG_TESTPA1       0x5A  // RFM69HW only
#define REG_TESTPA2       0x5C  // RFM69HW only
#define REG_TESTDAGC      0x6F

typedef enum {
    MODE_SLEEP = 0,     /**< Low power sleep mode */
    MODE_STANDBY = 1,   /**< Standby mode, configuration happens here */
    MODE_TX = 3,        /**< Transmit mode, starts transmission */
    MODE_RX = 4,        /**< Reception mode - listens for data */
} rfm69_mode_t;

typedef enum {
    IRQ_MODE_READY = (1 << 15),
    IRQ_RX_READY = (1 << 14),
    IRQ_TX_READY = (1 << 13),
    IRQ_PLL_LOCK = (1 << 12),
    IRQ_RSSI = (1 << 11),
    IRQ_TIMEOUT = (1 << 10),
    IRQ_AUTO_MODE = (1 << 9),
    IRQ_SYNC_ADDR_MATCH = (1 << 8),
    IRQ_FIFO_FULL = (1 << 7),
    IRQ_FIFO_NOT_EMPTY = (1 << 6),
    IRQ_FIFO_LEVEL = (1 << 5),
    IRQ_FIFO_OVERRUN = (1 << 4),
    IRQ_PACKET_SENT = (1 << 3),
    IRQ_PAYLOAD_READY = (1 << 2),
    IRQ_CRC_OK = (1 << 1),
} rfm69_int_t;

#define cs_set() IOd_SetLine(desc->cs_port, desc->cs_pad, 0)
#define cs_unset() IOd_SetLine(desc->cs_port, desc->cs_pad, 1)

/**
 * Write data to memory
 *`
 * @param desc  Device descriptor
 * @param addr  Address to start writing from
 * @param data  Data buffer
 * @param len   Amount of bytes to write
 */
static void write(const rfm69_desc_t *desc, uint8_t addr, const uint8_t *data, size_t len)
{
    cs_set();
    (void)SPId_Transceive(desc->spi_device, addr | 0x80);
    SPId_Send(desc->spi_device, data, len);
    cs_unset();
}

/**
 * Read data from memory
 *`
 * @param desc  Device descriptor
 * @param addr  Address to reading from
 * @param data  Data buffer
 * @param len   Amount of bytes to read
 */
static void read(const rfm69_desc_t *desc, uint8_t addr, uint8_t *data, size_t len)
{
    cs_set();
    (void)SPId_Transceive(desc->spi_device, addr & 0x7f);
    SPId_Receive(desc->spi_device, data, len);
    cs_unset();
}

/**
 * Write value to register
 *`
 * @param desc  Device descriptor
 * @param addr  Register address
 * @param value Value to be written
 */
static inline void writeReg(const rfm69_desc_t *desc, uint8_t addr, uint8_t value)
{
    write(desc, addr, &value, 1);
}

/**
 * Write value to 16 bit register
 *`
 * @param desc  Device descriptor
 * @param addr  Register address
 * @param value Value to be written
 */
static inline void writeReg16(const rfm69_desc_t *desc, uint8_t addr, uint16_t value)
{
    uint8_t data[2];
    data[0] = value >> 8;
    data[1] = value;
    write(desc, addr, data, sizeof(data));
}

/**
 * Read register value
 *
 * @param desc  Device descriptor
 * @param addr  Register address
 * @return register value
 */
static inline uint8_t readReg(const rfm69_desc_t *desc, uint8_t addr)
{
    uint8_t data;
    read(desc, addr, &data, 1);
    return data;
}

/**
 * Read 16 bit register value
 *
 * @param desc  Device descriptor
 * @param addr  Register address
 * @return register value
 */
static inline uint16_t readReg16(const rfm69_desc_t *desc, uint8_t addr)
{
    uint8_t data[2];
    read(desc, addr, data, 2);
    return data[0] << 8 | data[1];
}

/**
 * Modify content of the selected bits in register
 *
 * register = (register & mask) | bits;

 * @param desc  Device descriptor
 * @param addr  Register address
 * @param mask  Mask to be anded with the register value
 * @param bits  Value to be ored with the register value
 */
static inline void modifyReg(const rfm69_desc_t *desc, uint8_t addr, uint8_t mask, uint8_t bits)
{
    uint8_t reg = readReg(desc, addr);
    writeReg(desc, addr, (reg & mask) | bits);
}

/**
 * Wait until flag is set in IRQ register
 *
 * @param desc  Device descriptor
 * @param flag  flags to be checked (waits until any of flags is set)
 */
static void waitForIRQ(const rfm69_desc_t *desc, uint16_t flags)
{
    uint32_t start = millis();
    while (((readReg16(desc, REG_IRQFLAGS1) & flags) == 0) && ((millis() - start) < CMD_TIMEOUT_MS)) {
        ;
    }
}

/**
 * Control high power output (+20 dBm) mode of Hxx devices
 *
 * @note Before switching to receive mode, the high power must be disabled
 * @param state    Enable high power mode if true, else disable
 */
static void setHighPower(const rfm69_desc_t *desc, bool state)
{
    if (state) {
        writeReg(desc, REG_TESTPA1, 0x5D);
        writeReg(desc, REG_TESTPA2, 0x7C);
    } else {
        writeReg(desc, REG_TESTPA1, 0x55);
        writeReg(desc, REG_TESTPA2, 0x70);
    }
}

/**
 * Switch to a operational mode, wait until mode is ready if required
 *
 * @param desc  Device descriptor
 * @param mode  Required mode
 * @param wait  Wait for mode switch to be finished
 */
static void setMode(const rfm69_desc_t *desc, rfm69_mode_t mode, bool wait)
{
    writeReg(desc, REG_OPMODE, mode);
    if (desc->high_power) {
        setHighPower(desc, mode == MODE_TX);
    }

    if (wait) {
        waitForIRQ(desc, IRQ_MODE_READY);
    }
}

void RFM69_SetPowerDBm(rfm69_desc_t *desc, int8_t dBm)
{
    setMode(desc, MODE_STANDBY, true);
    writeReg(desc, REG_PARAMP, 0x0c);

    if (!desc->is_hxx) {
        if (dBm < -18) {
            dBm = -18;
        } else if (dBm > 13) {
            dBm = 13;
        }
        desc->high_power = false;
        writeReg(desc, REG_PALEVEL, (18 + dBm) | (1 << 7));
        writeReg(desc, 0x1A, REG_OCP);
        setHighPower(desc, false);
        return;
    }
    
    /* the antenna is connected to PA_BOOST pin, PA1 and/or PA2 must be used */
    if (dBm < -2) {
        dBm = -2;
    } else if (dBm > 20) {
        dBm = 20;
    }

    /* Set overcurrent protection and pa registers for 20dBm mode */
    if (dBm > 17) {
        writeReg(desc, REG_OCP, 0x0F); // Turn off the OCP
        desc->high_power = true;
    } else {
        writeReg(desc, REG_OCP, 0x1A); // Turn on the OCP
        setHighPower(desc, false);
        desc->high_power = false;
    }

    if (dBm <= 13) {
        writeReg(desc, REG_PALEVEL, (18 + dBm) | (1 << 6));
    } else if (dBm <= 17) {
        writeReg(desc, REG_PALEVEL, (14 + dBm) | (1 << 6) | (1 << 5));
    } else {
        writeReg(desc, REG_PALEVEL, (11 + dBm) | (1 << 6) | (1 << 5));
    }
}

void RFM69_SetFrameFormat(rfm69_desc_t *desc, const rfm69_frame_t *format)
{
    setMode(desc, MODE_STANDBY, true);

    // generic packet config
    writeReg(desc, REG_PACKETCONFIG1,
        (format->variable_len << 7) |
        (format->encoding << 5) |
        (format->crc << 4) |
        (format->filter.enable << 2));
    writeReg(desc, REG_PACKETCONFIG2, 0x02 | format->aes.enable);
    
    // (maximum) payload length
    writeReg(desc, REG_PAYLOADLENGTH, format->payload_len);
    desc->payload_len = format->payload_len ? 0 : format->payload_len;

    // Preamble length
    writeReg16(desc, REG_PREAMBLEMSB, format->preamble_len);

    // synchronization
    if (format->sync.len == 0) {
        writeReg(desc, REG_SYNCCONFIG, 0);
    } else {
        writeReg(desc, REG_SYNCCONFIG, (0x01 << 7) | (((format->sync.len - 1) & 0x07) << 3) |
            (format->sync.tolerance & 0x07));
        write(desc, REG_SYNCVALUE1, format->sync.value, format->sync.len);
    }

    // address filtering
    if (format->filter.enable) {
        writeReg(desc, REG_NODEADRS, format->filter.unicast);
        writeReg(desc, REG_BROADCASTADRS, format->filter.broadcast);
    }

    // encryption
    if (format->aes.enable) {
        write(desc, REG_AESKEY1, format->aes.key, 16);
    }
}

void RFM69_SetFrequencyHz(const rfm69_desc_t *desc, uint32_t freq_hz)
{
    uint32_t value = ((uint64_t)freq_hz * (1<<19))/FXOSC;
    uint8_t data[3];

    data[0] = value >> 16;
    data[1] = value >> 8;
    data[2] = value;
    setMode(desc, MODE_STANDBY, true);
    write(desc, REG_FRFMSB, data, sizeof(data));
}

void RFM69_SetRadioConfig(const rfm69_desc_t *desc, const rfm69_config_t *config)
{
    setMode(desc, MODE_STANDBY, true);
    RFM69_SetFrequencyHz(desc, config->frequency_hz);
    writeReg(desc, REG_DATAMODUL, config->modulation); // packet mode
    writeReg16(desc, REG_BITRATEMSB, FXOSC/config->bitrate_bps);
    writeReg16(desc, REG_FDEVMSB, ((uint64_t)config->freq_deviation_hz * (1 << 19)) / FXOSC);
    writeReg(desc, REG_RXBW, config->rx_bw | 0x40); // RX filter bandwidth, cutoff at 4%
}

bool RFM69_IsChannelEmpty(const rfm69_desc_t *desc, int8_t threshold)
{
    uint32_t start = millis();

    for (uint8_t i = 0; i < 3; i++) {
        writeReg(desc, REG_RSSICONFIG, 0x01); // start rssi measurement
        while (((readReg(desc, REG_RSSICONFIG) & 0x02) == 0) && ((millis() - start) < 10)) {
            ;
        }
        if (-readReg(desc, REG_RSSICONFIG)/2 >= threshold) {
            return false;
        }
    }
    return true;
}

void RFM69_Send(const rfm69_desc_t *desc, const uint8_t *data, uint8_t len)
{
    setMode(desc, MODE_STANDBY, true);
    /* clear fifo */
    writeReg16(desc, REG_IRQFLAGS1, IRQ_FIFO_OVERRUN | IRQ_RSSI);
    write(desc, REG_FIFO, data, len);

    setMode(desc, MODE_TX, true);
    waitForIRQ(desc, IRQ_PACKET_SENT);
    setMode(desc, MODE_STANDBY, false);
}

uint8_t RFM69_Receive(const rfm69_desc_t *desc, uint8_t *data, uint8_t max_len, int8_t *rssi)
{
    uint8_t len;

    if ((readReg16(desc, REG_IRQFLAGS1) & IRQ_PAYLOAD_READY) == 0) {
        return 0;  // no data available
    }

    if (rssi != NULL) {
        *rssi = readReg(desc, REG_RSSIVALUE);
    }

    if (desc->payload_len == 0) {
        len = readReg(desc, REG_FIFO);
    } else {
        len = desc->payload_len;
    }
    if (len > max_len) {
        len = max_len;
    }
    read(desc, REG_FIFO, data, len);

    return len;
}

void RFM69_StartReceiver(const rfm69_desc_t *desc)
{
    setMode(desc, MODE_RX, false);
}

/**
 * Initialize the RFM69 module
 *
 * @note SPI clock must be 10 MHz or lower
 */
bool RFM69_Init(rfm69_desc_t *desc, uint8_t spi_device, uint32_t cs_port,
        uint8_t cs_pad, uint32_t reset_port, uint8_t reset_pad,
        bool is_hxx)
{
    desc->spi_device = spi_device;
    desc->cs_port = cs_port;
    desc->cs_pad = cs_pad;
    desc->reset_port = reset_port;
    desc->reset_pad = reset_pad;
    desc->is_hxx = is_hxx;
    desc->high_power = false;
    desc->payload_len = 0;

    /* Reset the device */
    if (desc->reset_port != 0xff) {
        IOd_SetLine(desc->reset_port, desc->reset_pad, false);
        delay_ms(1);
        IOd_SetLine(desc->reset_port, desc->reset_pad, true);
        delay_ms(5);
    }

    /* Verify device signature */
    if (readReg(desc, REG_VERSION) != 0x24) {
        return false;
    }

    writeReg(desc, REG_OPMODE, MODE_STANDBY); // sequencer on, listen off, standby
    writeReg(desc, REG_AUTOMODES, 0x0);         // auto modes off
    writeReg(desc, REG_FIFOTHRESH, (1 << 7) | 0x0f); // FifoNotEmpty as tx start condition, FIFO threshold to defaul
    writeReg(desc, REG_LNA, 1 << 7);            // 50 ohm input impedance, auto LNA
    writeReg(desc, REG_RSSITHRESH, 0xE4);       // RSSI threshold = -114dBm
    writeReg(desc, REG_TESTLNA, 0x2D);          // higher LNA sensitivity
    writeReg(desc, REG_AFCFEI, 0x00); // AfcAuto off
    /*
     * DIO0=01, DIO4=01, ClkOut=off
     * Rx: DIO0 = PayloadReady, DIO4 = Rssi
     * Tx: DIO0 = TxReady, DIO4 = TxReady 
     */
    writeReg16(desc, REG_DIOMAPPING1, (0x01 << 14) | (0x01 << 6) | 0x07); 
    
    return true;
}

/** @} */
