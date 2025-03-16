/**
 * @file    ogntp.c
 * @brief   OGN Tracking Protocol implementation
 *
 * - The official description at http://wiki.glidernet.org/ogn-tracking-protocol is very sparse and
 * incorrect, unusable
 * - Official implementation is hard to read:
 * https://github.com/glidernet/diy-tracker/blob/master/ogn.h
 * - Modern re-implementation is bit better:
 * https://github.com/pjalocha/esp32-ogn-tracker/blob/master/main/ogn1.h
 * - OpenAce is actually quite nice:
 * https://github.com/rvt/OpenAce/blob/initial/src/lib/ogn/ace/ognpacket.hpp
 * - Some old proposition is here
 * https://docs.google.com/document/d/1lx5d6qcAk7sZULNXXTvC7bQsNCZKk73gKW74jdlIMA4/edit?tab=t.0#heading=h.4jkam575zkd3
 *
 * This library is blend of the above sources in attempt to make the OGN code much more readable
 */

#include <types.h>
#include "utils/utils.h"
#include "ogn_internal.h"
#include "ogntp.h"

/**
 * OGNTP frame format
 *
 * Most of the numeric value use a compression algorithm, to get value in unit mentioned in comment,
 * the binary data must be uncompressed first.
 */
typedef struct __attribute__((packed)) {
    union {
        uint32_t header_data;

        struct {
            uint32_t address  : 24; /**< Unique aircraft address */
            uint32_t addr_type: 2;  /**< 0 = random, 1 = ICAO, 2 = FLARM, 3 = OGN */
            uint32_t non_pos  : 1;  /**< payload type: 0 = position, 1 = Wind, Status... (see
                                       report_type) */
            uint32_t parity   : 1;  /**< Even parity for bits 0 to 27 */
            uint32_t relay    : 2;  /**< Number of times the frame was relayed, 0=direct */
            uint32_t encrypted: 1;  /**< 1 = Data encrypted or custom format */
            uint32_t emergency: 1;  /**< 1 = Aircraft in emergency, help requested */
        } header;
    };

    union {
        uint32_t data[4];

        struct {
            int32_t latitude    : 24; /**< Latitude in minutes * 10000 >> 3 */
            uint32_t time_s     : 6;  /**< UTC second, 0-59 */
            uint32_t fix_quality: 2;  /**< GPS fix quality, 0=no fix, 1=GPS, 2=D-GPS */
            int32_t longitude   : 24; /**< Longitude in minutes * 10000 >> 4 */
            uint32_t dop        : 6;  /**< GPS dilution of precision, 0.1 m */
            uint32_t
                baro_msb: 1; /**< Negated bit 8 of the altitude difference between baro and GPS */
            uint32_t fix_mode: 1;  /**< GPS fix type, 0=2D, 1=3D*/
            uint32_t altitude: 14; /**< Altitude in m */
            uint32_t speed   : 10; /**< Ground speed in 0.1 m/s */
            uint32_t
                turn_rate: 8; /**< Ground track turning rate in 0.1 deg/s, 0x80 for not available */
            uint32_t heading   : 10; /**< Ground track heading 0.1 deg deg */
            uint32_t climb_rate: 9;  /**< Climb rate from GNSS or pressure sensor in 0.1 m/s, 0x100
                                        for not available */
            uint32_t stealth_flag : 1; /**< Don't show in trackers */
            uint32_t aircraft_type: 4; /**< Type of the aircraft, see ogntp_aircraft_type */
            uint32_t baro_diff: 8; /**< Lower 8 bits of the difference between baro and GPS in m */
        } position;
    };

    uint8_t fec[6]; /**<  Forward error correction data using LDPC Gallager code */
} packet_v1_t;

/**
 * Encode latitude from decimal degrees to OGN format
 *
 * The OGN format unit is 0.0001/60 degrees (minutes multiplied by 10000)
 * divided by 8 to fit 24 bits
 */
static int32_t encodeLatitude(nmea_float_t latitude)
{
    int32_t result = (int64_t)latitude.num * 60 * 10000 / latitude.scale;
    return result >> 3;
}

/**
 * Encode longitude from decimal degrees to OGN format
 *
 * The OGN format unit is 0.0001/60 degrees (minutes multiplied by 10000)
 * divided by 16 to fit 24 bits
 */
static int32_t encodeLongitude(nmea_float_t longitude)
{
    int32_t result = (int64_t)longitude.num * 60 * 10000 / longitude.scale;
    return result >> 4;
}

/**
 * Encode altitude to OGN 14 bits format
 *
 * @param alt_m Altitude in meters, up to 0 to 61432 m
 */
static uint16_t encodeAltitude(int32_t alt_m)
{
    uint16_t value = alt_m > 0 ? alt_m : 0;
    return encodeToUint14(value);
}

/**
 * Encode speed to OGN 10 bits format
 *
 * @param speed_dms Speed in 0.1 m/s unit, 0 to 383.2 m/s
 */
static uint16_t encodeSpeed(int32_t speed_dms)
{
    uint16_t value = speed_dms > 0 ? speed_dms : 0;
    return encodeToUint10(value);
}

/**
 * Encode GPS DOP to OGN 4 bits format
 *
 * @param dop_dm    Unit of 0.1, 1.0 to "infinity"
 */
static uint8_t encodeDOP(uint8_t dop_d)
{
    return encodeToUint4(dop_d - 10);
}

/**
 * Encode heading to OGN format 360/1024 deg unit
 *
 * @para heading_ddeg   Heading in 0.1 degrees unit, 0 to 359.9
 */
static uint16_t encodeHeading(uint16_t heading_ddeg)
{
    return (((uint32_t)heading_ddeg << 10) + 180) / 3600;
}

/**
 * Encode turn rate to OGN 8 bit format
 *
 * @param rate_ddegs    Turn rate in 0.1 deg/s units, -47.2 to 47.2 deg/s
 */
static uint8_t encodeTurnRate(int16_t rate_ddegs)
{
    return encodeSignedToUint8(rate_ddegs);
}

/**
 * Encode climb rate to OGN 9 bit format
 *
 * @param rate_dms      Turn rate in 0.1 m/s units, -95.2 to 95.2
 */
static uint8_t encodeClimbRate(int16_t rate_dms)
{
    return encodeSignedToUint9(rate_dms);
}

/** Calculate parity bit (even parity) for header (first 28 bits)*/
static bool getParityBit(const packet_v1_t *packet)
{
    return count1s(packet->header_data & 0x07FFFFFF) & 0x01;
}

/**
 * Encode positional data into OGN packet structure
 *
 * @param packet    Encode data to this memory address
 * @param address   OGN address, 24 bits
 * @param gps       Current GPS data
 */
static void fillPositionPacket(packet_v1_t *packet, const ogntp_aircraft_t *aircraft,
    const gps_info_t *gps)
{
    memset(packet, 0x0, sizeof(packet_v1_t));
    packet->header.emergency = 0;
    packet->header.encrypted = 0;
    packet->header.relay = 0;   // direct packet
    packet->header.non_pos = 0; // positional data
    packet->header.addr_type = aircraft->addr_type;
    packet->header.address = aircraft->address & 0x00ffffff;
    packet->position.aircraft_type = aircraft->type;
    packet->position.stealth_flag = 0;

    packet->position.latitude = encodeLatitude(gps->latitude);
    packet->position.longitude = encodeLongitude(gps->longitude);
    packet->position.altitude = encodeAltitude((gps->altitude_dm + 5) / 10);
    packet->position.speed = encodeSpeed(gps->speed_dms);
    packet->position.heading = encodeHeading(gps->heading_ddeg);
    packet->position.dop = encodeDOP(gps->hdop_d);
    packet->position.fix_quality = gps->fix_quality > 0x03 ? 0x03 : gps->fix_quality;
    packet->position.fix_mode = gps->is_3d_fix;
    packet->position.time_s = gps->time.second;

    packet->position.turn_rate = 0x80;   // not available
    packet->position.climb_rate = 0x100; // not available
    // no barometer present
    packet->position.baro_msb = 0;
    packet->position.baro_diff = 0;

    packet->header.parity = getParityBit(packet);
}

void OGNTP_EncodePosition(uint8_t buffer[26], const ogntp_aircraft_t *aircraft,
    const gps_info_t *gps)
{
    packet_v1_t packet;
    fillPositionPacket(&packet, aircraft, gps);
    whitenPayload((uint8_t *)packet.data);
    getFCS((uint8_t *)&packet, packet.fec);
    memcpy(buffer, &packet, sizeof(packet));
}
