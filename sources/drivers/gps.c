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
 * @file    drivers/gps.h
 * @brief   Driver for gps receiver (sleep/wake up implemented for SIM28)
 *
 * @addtogroup drivers
 * @{
 */

#include "hal/uart.h"
#include "hal/io.h"
#include "utils/ringbuf.h"
#include "utils/time.h"
#include "modules/log.h"

#include "gps.h"

#define GPS_DEVICE USART_GPS_TX

static ring_t gpsi_ringbuf;
/** GPS data are valid when set to 0x03, 0 invalid, 1 gga, 2 rmc */
static uint8_t gpsi_data_valid = 0;
static gps_info_t gpsi_info;

/**
 * Callback for data received from GPS
 *
 * Process received NMEA message and store data to external structure
 */
static void Gpsi_RxCb(uint8_t c)
{
    Ring_Push(&gpsi_ringbuf, c);
}

/**
 * Convert nmea float type to decimal with required scale (e.g. 123,456 to
 * 1234 for scale 10)
 *
 * @param f     Float number
 * @param scale Required scale
 * @return  Converted number
 */
static int32_t Gpsi_NmeaF2Dec(const nmea_float_t *f, int32_t scale)
{
    if (f->scale < scale) {
        return f->num * (scale/f->scale);
    }
    return f->num/(f->scale/scale);
}

/**
 * Convert nmea date and time into unix timestamp
 *
 * @param time      Time structure
 * @param date      Date structure
 * @return  Unix formated timestamp (seconds since 1.1.1970)
 */
static time_t Gpsi_Nmea2Timestamp(const nmea_time_t *time,
        const nmea_date_t *date)
{
    struct tm tmstruct = {0};

    tmstruct.tm_sec = time->second;
    tmstruct.tm_min = time->minute;
    tmstruct.tm_hour = time->hour;
    tmstruct.tm_mday = date->day;
    tmstruct.tm_mon = date->month - 1;
    tmstruct.tm_year = date->year + 100;

    return mktime(&tmstruct);
}

static bool Gpsi_ProcessRmc(const char *msg, gps_info_t *info)
{
    nmea_rmc_t rmc;

    if (!Nmea_ParseRmc(msg, &rmc)) {
        return false;
    }

    if (!rmc.valid) {
        return false;
    }

    info->speed_dmh = Gpsi_NmeaF2Dec(&rmc.speed_kmh, 10);
    info->lat = rmc.lat;
    info->lon = rmc.lon;
    info->time = Gpsi_Nmea2Timestamp(&rmc.fix_time, &rmc.date);
    return true;
}

static bool Gpsi_ProcessGga(const char *msg, gps_info_t *info)
{
    nmea_gga_t gga;

    if (!Nmea_ParseGga(msg, &gga)) {
        return false;
    }

    if (gga.quality == 0) {
        return false;
    }

    info->satellites = gga.satellites;
    info->lat = gga.lat;
    info->lon = gga.lon;
    info->hdop_dm = Gpsi_NmeaF2Dec(&gga.hdop, 10);
    info->altitude_dm = Gpsi_NmeaF2Dec(&gga.altitude_m, 10);
    return true;
}

void Gps_Sleep(void)
{
    gpsi_data_valid = 0;
    UARTd_Puts(GPS_DEVICE, "$PMTK161,0*28\r\n");
}

void Gps_WakeUp(void)
{
    /* just random command to wake device up (acking nonexisting message) */
    UARTd_Puts(GPS_DEVICE, "$PMTK001,604,3*32\r\n");
}

gps_info_t *Gps_Get(void)
{
    if (gpsi_data_valid == 0x03) {
        return &gpsi_info;
    }
    return NULL;
}

void Gps_InvalidateData(void)
{
    gpsi_data_valid = 0;
}

gps_info_t *Gps_Loop(void)
{
    static uint8_t prev = 0;

    while (!Ring_Empty(&gpsi_ringbuf)) {
        const char *msg = Nmea_AddChar(Ring_Pop(&gpsi_ringbuf));
        if (msg == NULL) {
            continue;
        }

        Log_Debug("GPS", msg);
        switch (Nmea_GetSentenceType(msg)) {
            case NMEA_SENTENCE_GGA:
                /* Main source of data, sets data validity */
                if (Gpsi_ProcessGga(msg, &gpsi_info)) {
                    gpsi_data_valid |= 0x01;
                }
                break;
            case NMEA_SENTENCE_RMC:
                if (Gpsi_ProcessRmc(msg, &gpsi_info)) {
                    gpsi_data_valid |= 0x02;
                }
                break;
            default:
                break;
        }
    }

    if (gpsi_data_valid != prev && gpsi_data_valid == 0x03) {
        gpsi_info.timestamp = millis();
        prev = gpsi_data_valid;
        return &gpsi_info;
    }

    prev = gpsi_data_valid;
    return NULL;
}

void Gps_Init(void)
{
    static char gpsi_buf[32];

    Ring_Init(&gpsi_ringbuf, gpsi_buf, sizeof(gpsi_buf));
    UARTd_SetRxCallback(GPS_DEVICE, Gpsi_RxCb);
}

/** @} */
