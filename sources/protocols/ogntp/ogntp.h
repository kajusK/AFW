/**
 * @file    ogntp.h
 * @brief   OGN Tracking Protocol implementation
 *
 * Radio settings:
 *   100kbps bit rate
 *   +-50 kHz deviation
 *   243.3 kHz rxBandwidth
 *   NRZ encoding
 *   8 bits preamble length (0xAA)
 *   FSK modulation shaping BT=0.5
 */

#ifndef _PROTOCOL_OGNTP_H_
#define _PROTOCOL_OGNTP_H_

#include <types.h>
#include "drivers/gps.h"
#include "utils/nav.h"

/** Length of the OGN frame in bytes */
#define OGNTP_FRAME_BYTES 52

/** OGNTP sync frame, 0x0AF3656C encoded in Manchester */
#define OGNTP_SYNC { 0xAA, 0x66, 0x55, 0xA5, 0x96, 0x99, 0x96, 0x5A }

/** Length of the OGNTP frame transmission in ms */
#define OGNTP_TX_LEN_MS 5

/** Slot 0 timing, delay since the GPS PPS pulse */
#define OGNTP_SLOT0_START_MS 400
#define OGNTP_SLOT0_END_MS   800

/** Slot 1 timing, delay since the GPS PPS pulse, can happen after next pps pulse */
#define OGNTP_SLOT1_START_MS 800
#define OGNTP_SLOT1_END_MS   1200

/**
 * List of available aircraft types
 * Based on http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
 */
typedef enum {
    OGNTP_AIRCRAFT_UNKNOWN = 0,
    OGNTP_AIRCRAFT_GLIDER = 1,
    OGNTP_AIRCRAFT_TOW_PLANE = 2,
    OGNTP_AIRCRAFT_HELICOPTER = 3,
    OGNTP_AIRCRAFT_PARACHUTE = 4, // often mixed with drop plane
    OGNTP_AIRCRAFT_DROP_PLANE = 5,
    OGNTP_AIRCRAFT_HANG_GLIDER = 6,
    OGNTP_AIRCRAFT_PARA_GLIDER = 7,
    OGNTP_AIRCRAFT_POWERED = 8,
    OGNTP_AIRCRAFT_JET = 9,
    OGNTP_AIRCRAFT_UFO = 10,
    OGNTP_AIRCRAFT_BALLOON = 11,
    OGNTP_AIRCRAFT_AIRSHIP = 12,
    OGNTP_AIRCRAFT_UAV = 13,
    OGNTP_AIRCRAFT_GROUND_OBJECT = 14,
    OGNTP_AIRCRAFT_STATIC_OBJECT = 15
} ogntp_aircraft_type_t;

/** Address type to be reported */
typedef enum {
    OGNTP_ADDRESS_RANDOM = 0x0, /**< Random value, for temporary use */
    OGNTP_ADDRESS_ICAO = 0x1,   /**< ICAO assigned identifier */
    OGNTP_ADDRESS_FLAGM = 0x2,  /**< ID of the FLARM device */
    OGNTP_ADDRESS_OGN =
        0x3, /**< Address registered in http://ddb.glidernet.org/, pick unused value and register */
} ogntp_address_type_t;

/** Identification of the OGN transmitter */
typedef struct {
    uint32_t address; /**< 24 bits of the aircraft address */
    ogntp_address_type_t addr_type;
    ogntp_aircraft_type_t type;
} ogntp_aircraft_t;

/**
 * Create an OGNTP position message
 *
 * @param buffer    Buffer to store message to
 * @param aircraft  Aircraft identification
 * @param gps       GPS data to create frame from
 */
void OGNTP_EncodePosition(uint8_t buffer[OGNTP_FRAME_BYTES], const ogntp_aircraft_t *aircraft,
    const gps_info_t *gps);

/**
 * Get frequency to transmit on in given timeslot
 *
 * The OGNTP has two time slots defined, each timeslot can have a different
 * transmit frequency.
 *
 * @param region          The region to select frequency plan for
 * @param slot            Timeslot to use (0 or 1)
 * @param timestamp_utc   Current UTC time as unix timestamp, used for frequency hoping
 * @return Transmit frequency in Hz
 */
uint32_t OGNTP_GetFrequencyHz(nav_region_t region, uint8_t slot, uint32_t timestamp_utc);

#endif
