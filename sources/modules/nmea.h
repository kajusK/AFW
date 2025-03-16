/**
 * @file    modules/nmea.h
 * @brief   NMEA 0183 messages parser
 */

#ifndef __MODULES_NMEA_H
#define __MODULES_NMEA_H

#include <types.h>

/** Maximum value that can be stored in SNR in satellite info */
#define MAX_SV_SNR 100

/** Date keeping structure, -1 means field is not valid */
typedef struct {
    int8_t day;
    int8_t month;
    int8_t year;
} nmea_date_t;

/** Time keeping structure, -1 means field is not valid */
typedef struct {
    int8_t hour;
    int8_t minute;
    int8_t second;
    int32_t micros;
} nmea_time_t;

/* Float number representation, divide num by scale to get float number */
typedef struct {
    int32_t num;
    int32_t scale;
} nmea_float_t;

/** Satellite in view info */
typedef struct {
    uint8_t prn;       /**< Satellite PRN number, 1-32 for gps */
    uint8_t elevation; /**< 0-90 degrees */
    uint16_t azimuth;  /**< 0-359 degrees */
    uint8_t snr;       /**< 0-99 dB */
} nmea_sv_info_t;

typedef struct {
    nmea_time_t fix_time;       /**< UTC time of GPS fix */
    bool valid;                 /**< Validity of data, do not use when invalid */
    nmea_float_t lat;           /**< Latitude in decimal degrees */
    nmea_float_t lon;           /**< Longitude in decimal degrees */
    nmea_float_t speed_ms;      /**< Speed over ground in m/s */
    nmea_float_t heading;       /**< Heading over ground in degrees */
    nmea_date_t date;           /**< Current date */
    nmea_float_t mag_variation; /**< Magnetic variation in degrees */
} nmea_rmc_t;

typedef struct {
    nmea_time_t fix_time;           /**< UTC time of GPS fix */
    nmea_float_t lat;               /**< Latitude in decimal degrees */
    nmea_float_t lon;               /**< Longitude in decimal degrees */
    uint8_t quality;                /**< Fix quality 0 Fix not valid, 1 GPS, 2 Differential GPS */
    uint8_t satellites;             /**< Number of satellites in use */
    nmea_float_t hdop;              /**< Horizontal dilution of precision, 1.0 to infinity */
    nmea_float_t altitude_m;        /**< Height above MSL in meters */
    nmea_float_t above_ellipsoid_m; /**< Geoidal separation measures in meters */
} nmea_gga_t;

typedef struct {
    uint8_t messages;     /**< Total number of GSV messages for this cycle */
    uint8_t msg_id;       /**< Current message number out of all gsv messages */
    uint8_t visible;      /**< Total number of visible satellites */
    uint8_t count;        /**< Amount of SVs in this message */
    nmea_sv_info_t sv[4]; /**< List of received satellites */
} nmea_gsv_t;

typedef enum {
    NMEA_SENTENCE_UNKNOWN,
    NMEA_SENTENCE_RMC,
    NMEA_SENTENCE_GGA,
    NMEA_SENTENCE_GSV,
} nmea_type_t;

/**
 * Check if the message has a valid checksum, $ at the beginning is optional
 *
 * @param msg   Message to be checked
 * @return      True if checksum is valid
 */
bool Nmea_VerifyChecksum(const char *msg);

/**
 * Verify the message contains valid NMEA structure
 *
 * @param msg   Message to be checked
 * @return      True if valid (and checksum matches if present)
 */
bool Nmea_VerifyMessage(const char *msg);

/**
 * Parse NMEA RMC message into structure
 *
 * @param msg   Message
 * @param rmc   Structure to parse data to
 * @return True if succeeded
 */
bool Nmea_ParseRmc(const char *msg, nmea_rmc_t *rmc);

/**
 * Parse NMEA GGA message into structure
 *
 * @param msg   Message
 * @param rmc   Structure to parse data to
 * @return True if succeeded
 */
bool Nmea_ParseGga(const char *msg, nmea_gga_t *gga);

/**
 * Parse NMEA GSV message into structure
 *
 * @param msg   Message
 * @param rmc   Structure to parse data to
 * @return True if succeeded
 */
bool Nmea_ParseGsv(const char *msg, nmea_gsv_t *gsv);

/**
 * Get type of the NMEA message
 * @param msg   Message
 * @return Message type
 */
nmea_type_t Nmea_GetSentenceType(const char *msg);

/**
 * Add character to internal buffer and detect complete NMEA message
 *
 * @param c     Character to add
 * @return  Pointer to string with NMEA message or NULL
 */
const char *Nmea_AddChar(char c);

#endif
