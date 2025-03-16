/**
 * @file    ogntp.h
 * @brief   OGN Tracking Protocol implementation
 */

#ifndef _PROTOCOL_OGNTP_H_
#define _PROTOCOL_OGNTP_H_

#include <types.h>
#include "drivers/gps.h"

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

/** Length of the OGN frame in bytes */
#define OGNTP_FRAME_BYTES 52

/**
 * Radio settings:
 *   100kbps bit rate
 *   +-50 kHz deviation
 *   243.3 kHz rxBandwidth
 *   NRZ encoding
 *   8 bits minimum preamble length
 *   FSK modulation shaping BT=0.5
 *   Preamble length = 8 bits
 */

/** OGNTP sync frame, 0x0AF3656C encoded in Manchester */
#define OGNTP_SYNC { 0xAA, 0x66, 0x55, 0xA5, 0x96, 0x99, 0x96, 0x5A }

/** OGNTP center frequency for Europe - there are actually two channels, let's keep this simple */
#define OGNTP_FREQUENCY_HZ 868200000

/**
 * Create an OGNTP position message
 *
 * @param buffer    Buffer to store message to
 * @param aircraft  Aircraft identification
 * @param gps       GPS data to create frame from
 */
void OGNTP_EncodePosition(uint8_t buffer[OGNTP_FRAME_BYTES], const ogntp_aircraft_t *aircraft,
    const gps_info_t *gps);

#endif
