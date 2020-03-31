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
 * @file    modules/nmea.c
 * @brief   NMEA 0183 messages parser
 *
 * NMEA message format taken from http://aprs.gids.nl/nmea/
 *
 * @addtogroup modules
 * @{
 */

#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "modules/nmea.h"

#define NMEA_MAX_MSG_LEN 82
#define Nmeai_IsEnd(c)  ((c) == ',' || (c) == '*' || (c) == '\0')

/**
 * Convert one letter hex value to decimal number
 *
 * @param c     Letter to be converted
 * @return 0 to 15, if different value is returned, c was not in [0-9,a-f,A-F]
 */
static uint8_t Nmeai_Hex2Dec(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return c - 'A' + 10;
}

/**
 * Convert string to number
 *
 * Converts first len characters or up to first non digit one
 *
 * @param str       Str to conver, will point to char after last digit after return
 * @param len       Amount of characters to convert
 * @return  Converted value
 */
static uint32_t Nmeai_Str2Dec(const char **str, uint8_t len)
{
    uint32_t num = 0;

    while (len-- != 0) {
        if (!isdigit((unsigned char)**str)) {
            return num;
        }
        num = (num * 10) + (*(*str)++ - '0');
    }
    return num;
}

/**
 * Convert nmea lat/lon degrees minutes format to decimal degrees
 *
 * @param f     Float number to be converted (4912.1234), result stored here (49.xxxxx)
 */
static void Nmeai_Float2DecDeg(nmea_float_t *f)
{
    int32_t min;
    int32_t deg;

    f->scale *= 100;
    deg = f->num / f->scale;
    min = f->num - deg*f->scale;

    f->num = deg*f->scale + min*10/6;
}

/**
 * Helper function for Nmeai_Scan, processes single item from NMEA message
 *
 * @param msg       Remaining part of nmea message
 * @param format    Single character describing format converted (@see Nmeai_Scan)
 * @param ap        Initialized va_list (variable arguments)
 * @return          NULL when failed or pointer to next character in msg buffer
 */
static const char *Nmeai_ScanHelper(const char *msg, char format, va_list *ap)
{
    int i;

    switch (format) {
        /* ignored character */
        case '_':
            if (!Nmeai_IsEnd(*msg)) {
                while (!Nmeai_IsEnd(*msg++)) {
                    ;
                }
            }
            break;

        /* single character */
        case 'c': {
            char value = '\0';
            if (!Nmeai_IsEnd(*msg)) {
                value = *msg++;
            }

            *va_arg(*ap, char *) = value;
            } break;

        /* direction N,S,E or W */
        case 'D': {
            int8_t value = 0;

            if (!Nmeai_IsEnd(*msg)) {
                if (*msg == 'N' || *msg == 'E') {
                    value = 1;
                } else if (*msg == 'S' || *msg == 'W') {
                    value = -1;
                } else {
                    return NULL;
                }
                msg++;
            }
            *va_arg(*ap, char *) = value;
            } break;

        /* string */
        case 's': {
            char *str;
            str = va_arg(*ap, char *);

            while (!Nmeai_IsEnd(*msg)) {
                *str++ = *msg++;
            }
            *str = '\0';
            } break;

        /* positive integer 05, 1234,... */
        case 'i': {
            int value = -1;
            if (!Nmeai_IsEnd(*msg)) {
                value = Nmeai_Str2Dec(&msg, 10);
            }
            *va_arg(*ap, int *) = value;
            } break;

        /* float 123.456 */
        case 'p':
        case 'f': {
            int32_t scale = 1;
            int32_t value = 0;
            int32_t sign = 1;

            if (!Nmeai_IsEnd(*msg)) {
                if (*msg == '+') {
                    msg++;
                }
                if (*msg == '-') {
                    sign = -1;
                    msg++;
                }
                value = Nmeai_Str2Dec(&msg, 10);
                if (*msg == '.') {
                    msg++;
                    i = 0;
                    scale = 1;
                    while (isdigit((unsigned char) *(msg + i))) {
                        i++;
                        scale *= 10;
                    }
                    value = value*scale + Nmeai_Str2Dec(&msg, i);
                }
            }
            nmea_float_t *f = va_arg(*ap, nmea_float_t *);
            f->num = sign*value;
            f->scale = scale;
            /* convert coordinates to decimal degrees */
            if (format == 'p') {
                Nmeai_Float2DecDeg(f);
            }
            } break;

        /* date 110122 = 11th of January, 2022 */
        case 'd': {
            uint8_t d = -1, m = -1, y = -1;
            if (!Nmeai_IsEnd(*msg)) {
                for (i = 0; i < 6; i++) {
                    if (!isdigit((unsigned char)*(msg + i))) {
                        return NULL;
                    }
                }

                d = Nmeai_Str2Dec(&msg, 2);
                m = Nmeai_Str2Dec(&msg, 2);
                y = Nmeai_Str2Dec(&msg, 2);
            }
            nmea_date_t *date = va_arg(*ap, nmea_date_t *);
            date->day = d;
            date->month = m;
            date->year = y;
            } break;

        /* time 112233 (11:22:33) or 112233.15 (11:22:33.15) */
        case 't': {
            int8_t hour = -1, min = -1, sec = -1;
            int32_t micros = 0;

            if (!Nmeai_IsEnd(*msg)) {
                for (i = 0; i < 6; i++) {
                    if (!isdigit((unsigned char)*(msg + i))) {
                        return NULL;
                    }
                }

                hour = Nmeai_Str2Dec(&msg, 2);
                min = Nmeai_Str2Dec(&msg, 2);
                sec = Nmeai_Str2Dec(&msg, 2);

                /* optional fraction time */
                if (*msg == '.') {
                    uint32_t scale = 1000000;
                    msg++;
                    i = 0;
                    while (isdigit((unsigned char)*(msg + i))) {
                        i++;
                        scale /= 10;
                    }
                    micros = Nmeai_Str2Dec(&msg, i) * scale;
                }
            }
            nmea_time_t *time = va_arg(*ap, nmea_time_t *);
            time->hour = hour;
            time->minute = min;
            time->second = sec;
            time->micros = micros;
            } break;

        case '\0':
            break;
    }

    return msg;
}

/**
 * Scanf like function for parsing nmea sentences
 *
 * @param msg       NMEA message
 * @param format
 * Supported format specifiers are:
 *  c - single character (char *)
 *  D - direction (NSEW) (int8_t *)
 *  i - positive integer (0, 05, 123,..) (int *)
 *  s - string (char *), pointed buffer must be long enough for string and '\0'
 *  f - floats (123.456)
 *  p - latitude/longitude (1245.1234 = 12°45.1234', will convert to decimal degrees)
 *  d - date (110112 = 11th January 2012) (nmea_date_t *)
 *  t - time (111213 or 111213.1423) (nmea_time_t *)
 *  _ - ignored field
 * @param ...  Variable argument list as specified by format
 * @return  True if succeeded, false if msg does not correspond to format
 */
static bool Nmeai_Scan(const char *msg, const char *format, ...)
{
    bool ret = true;
    va_list ap;
    va_start(ap, format);

    if (*msg == '$') {
        msg++;
    }

    while (*format != '\0') {
        msg = Nmeai_ScanHelper(msg, *format++, &ap);
        /* failed to parse field */
        if (msg == NULL) {
            ret = false;
            break;
        }

        if (*msg == ',') {
            msg++;
            continue;
        }

        /* end of message */
        if (*msg == '*' || *msg == '\0') {
            break;
        }
    }

    if (*format != '\0') {
        ret = false;
    }
    if (*msg != '*' && *msg != '\0') {
        ret = false;
    }

    va_end(ap);
    return ret;
}

bool Nmea_VerifyChecksum(const char *msg)
{
    uint8_t checksum = 0;

    if (*msg == '$') {
        msg++;
    }

    /* Checksum is xor of all bytes between $ and * in the message */
    while (*msg != '*' && *msg != '\0') {
        checksum ^= *msg++;
    }

    if (strlen(msg++) < 2) {
        return false;
    }

    if ((Nmeai_Hex2Dec(msg[0]) << 4 | Nmeai_Hex2Dec(msg[1])) == checksum) {
        return true;
    }
    return false;
}

bool Nmea_VerifyMessage(const char *msg)
{
    /* max length is limited by standard, min by common sense */
    if (strlen(msg) < 5 || strlen(msg) > NMEA_MAX_MSG_LEN) {
        return false;
    }

    /* if checkum is present, it must match */
    for (size_t i = 0; i < strlen(msg); i++) {
        if (msg[i] != '*') {
            continue;
        }
        if (i != strlen(msg) - 3 || !Nmea_VerifyChecksum(msg)) {
            return false;
        }
        break;
    }

    /* message must start with $ */
    if (*msg != '$') {
        return false;
    }
    return true;
}

bool Nmea_ParseRmc(const char *msg, nmea_rmc_t *rmc)
{
    char type[6];
    int8_t dir_lat, dir_lon, dir_var;
    char c;
    bool ret;
    int32_t div;

    if (!Nmea_VerifyMessage(msg)) {
        return false;
    }
    /* $GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68 */
    ret = Nmeai_Scan(msg, "stcpDpDffdfD",
            type, &rmc->fix_time, &c, &rmc->lat, &dir_lat, &rmc->lon, &dir_lon,
            &rmc->speed_kmh, &rmc->course, &rmc->date,
            &rmc->mag_variation, &dir_var);
    if (!ret || strncmp(type+2, "RMC", 3) != 0) {
        return false;
    }

    rmc->valid = c == 'A';
    rmc->lat.num *= dir_lat;
    rmc->lon.num *= dir_lon;
    rmc->mag_variation.num *= dir_var;

    /* convert from knots to kmh */
    if (rmc->speed_kmh.scale > 100) {
        div = rmc->speed_kmh.scale/100;
        rmc->speed_kmh.scale /= div;
        rmc->speed_kmh.num /= div;
    } else {
        div = 100/rmc->speed_kmh.scale;
        rmc->speed_kmh.scale *= div;
        rmc->speed_kmh.num *= div;
    }
    rmc->speed_kmh.num *= 1852;
    rmc->speed_kmh.num += 500; /* rounded, same as adding 0.5 */
    rmc->speed_kmh.num /= 1000; /* divided by 1000 to get to original range */

    return true;
}

bool Nmea_ParseGga(const char *msg, nmea_gga_t *gga)
{
    char type[6];
    int8_t dir_lat, dir_lon;
    char alt_unit, ellipsoid_unit;
    bool ret;
    int satellites, quality;

    if (!Nmea_VerifyMessage(msg)) {
        return false;
    }
    /* $GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76 */
    ret = Nmeai_Scan(msg, "stpDpDiiffcfc__",
            type, &gga->fix_time, &gga->lat, &dir_lat, &gga->lon, &dir_lon,
            &quality, &satellites, &gga->hdop, &gga->altitude_m, &alt_unit,
            &gga->above_ellipsoid_m, &ellipsoid_unit);
    if (!ret || strncmp(type+2, "GGA", 3) != 0) {
        return false;
    }

    gga->quality = quality;
    gga->satellites = satellites;
    gga->lat.num *= dir_lat;
    gga->lon.num *= dir_lon;

    return true;
}

nmea_type_t Nmea_GetSentenceType(const char *msg)
{
    /* skip $GP part of the message */
    msg += 3;
    if (strncmp("RMC", msg, 3) == 0) {
        return NMEA_SENTENCE_RMC;
    }
    if (strncmp("GGA", msg, 3) == 0) {
        return NMEA_SENTENCE_GGA;
    }

    return NMEA_SENTENCE_UNKNOWN;
}

const char *Nmea_AddChar(char c)
{
    static char buf[NMEA_MAX_MSG_LEN];
    static uint8_t pos = 0;

    if (pos == 0 && c != '$') {
        return NULL;
    }

    if (pos >= NMEA_MAX_MSG_LEN) {
        pos = 0;
        return NULL;
    }

    if (c == '\n' || c == '\r') {
        buf[pos] = '\0';
        pos = 0;
        return buf;
    }
    buf[pos++] = c;
    return NULL;
}

/** @} */
