/*
 * Copyright (C) 2019 Jakub Kaderka
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
 * @file    modules/nmea.h
 * @brief   NMEA 0183 messages parser
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_NMEA_H
#define __MODULES_NMEA_H

#include <types.h>

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

typedef struct {
    int32_t num;
    int32_t scale;
} nmea_float_t;

typedef struct {
    nmea_time_t fix_time;
    bool valid;
    nmea_float_t lat;   /* in decimal degrees */
    nmea_float_t lon;   /* in decimal degrees */
    nmea_float_t speed_kmh;
    nmea_float_t course;
    nmea_date_t date;
    nmea_float_t mag_variation;
} nmea_rmc_t;

typedef struct {
    nmea_time_t fix_time;
    nmea_float_t lat; /* in decimal degrees */
    nmea_float_t lon; /* in decimal degrees */
    uint8_t quality;
    uint8_t satellites;
    nmea_float_t hdop;
    nmea_float_t altitude_m;
    nmea_float_t above_ellipsoid_m;
} nmea_gga_t;

typedef enum {
    NMEA_SENTENCE_UNKNOWN,
    NMEA_SENTENCE_RMC,
    NMEA_SENTENCE_GGA,
} nmea_type_t;

/**
 * Check if the message has a valid checksum, $ at the beginning is optional
 *
 * @param msg   Message to be checked
 * @return      True if checksum is valid
 */
extern bool Nmea_VerifyChecksum(const char *msg);

/**
 * Verify the message contains valid NMEA structure
 *
 * @param msg   Message to be checked
 * @return      True if valid (and checksum matches if present)
 */
extern bool Nmea_VerifyMessage(const char *msg);

/**
 * Parse NMEA RMC message into structure
 *
 * @param msg   Message
 * @param rmc   Structure to parse data to
 * @return True if succeeded
 */
extern bool Nmea_ParseRmc(const char *msg, nmea_rmc_t *rmc);

/**
 * Parse NMEA GGA message into structure
 *
 * @param msg   Message
 * @param rmc   Structure to parse data to
 * @return True if succeeded
 */
extern bool Nmea_ParseGga(const char *msg, nmea_gga_t *gga);

/**
 * Get type of the NMEA message
 * @param msg   Message
 * @return Message type
 */
extern nmea_type_t Nmea_GetSentenceType(const char *msg);

/**
 * Add character to internal buffer and detect complete NMEA message
 *
 * @param c     Character to add
 * @return  Pointer to string with NMEA message or NULL
 */
extern const char *Nmea_AddChar(char c);

#endif

/** @} */
