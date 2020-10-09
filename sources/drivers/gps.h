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

#ifndef __DRIVERS_GPS_H
#define __DRIVERS_GPS_H

#include <types.h>
#include <time.h>
#include "utils/ringbuf.h"
#include "modules/nmea.h"

typedef struct {
    nmea_float_t lat;   /**< In decimal degrees */
    nmea_float_t lon;   /**< In deciaml degrees */
    int32_t altitude_dm;/**< Altitude in dm */
    int32_t speed_dmh;  /**< speed in dm/s */
    int32_t hdop_dm;    /**< location precision */
    uint8_t satellites; /**< Visible satellites */
    time_t time;        /**< Unix time (s since 1.1.1970) of the data received */
    uint32_t timestamp; /**< Millis timestamp when the gps fix was obtained */
} gps_info_t;

typedef struct {
    uint8_t visible;    /**< Total number of satellites in view */
    uint8_t count;      /**< Amount of valid records in sat array */
    nmea_sv_info_t sat[10];
} gps_sat_t;

/** GPS device description */
typedef struct {
    uint8_t uart_device;    /**< UART device to use for GPS connection */
    ring_t ringbuf;         /**< Ringbuffer to store received data */
    char buf[32];           /**< Data storage for ringbuffer */

    /** GPS data are valid when set to 0x03, 0 invalid, 1 gga, 2 rmc */
    uint8_t data_valid;
    gps_info_t info;
    gps_sat_t sat;
} gps_desc_t;

/**
 * Put GPS device into low power mode (invalidates currently stored gps info)
 *
 * @param desc        Device descriptor
 */
extern void Gps_Sleep(gps_desc_t *desc);

/**
 * Wake up GPS device from sleep mode
 *
 * @param desc        Device descriptor
 */
extern void Gps_WakeUp(const gps_desc_t *desc);

/**
 * Get GPS data
 *
 * @param desc        Device descriptor
 * @return Data if any or NULL if no data received yet
 */
extern const gps_info_t *Gps_Get(gps_desc_t *desc);

/**
 * Get GPS satellite info
 *
 * @param desc        Device descriptor
 * @return Pointer to satellite data
 */
extern const gps_sat_t *Gps_GetSat(gps_desc_t *desc);

/**
 * Reset GPS recorded data to wait for brand new in loop/get functions
 *
 * @param desc        Device descriptor
 */
extern void Gps_InvalidateData(gps_desc_t *desc);

/**
 * Process received GPS data
 *
 * Should be called periodically as it fetches the data from uart buffer
 *
 * @param desc        Device descriptor
 * @return data If have valid GPS data or NULL
 */
extern const gps_info_t *Gps_Loop(gps_desc_t *desc);

/**
 * Initialize GPS module
 *
 * Due to implementation limitations, only one device could be used (
 * the receiving function is using descriptor provided in last Gps_Init call)
 *
 * @param [out] desc        Device descriptor
 * @param uart_device       UART device on which the GPS is connected
 */
extern void Gps_Init(gps_desc_t *desc, uint8_t uart_device);

#endif

/** @} */
