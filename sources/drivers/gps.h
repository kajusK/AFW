/**
 * @file    drivers/gps.h
 * @brief   Driver for gps receiver (sleep/wake up implemented for SIM28)
 */

#ifndef __DRIVERS_GPS_H
#define __DRIVERS_GPS_H

#include <types.h>
#include <time.h>
#include "utils/ringbuf.h"
#include "protocols/nmea.h"

typedef struct {
    uint32_t timestamp;     /**< Millis timestamp when the gps fix was obtained */
    nmea_date_t date;       /**< UTC date of the fix */
    nmea_time_t time;       /**< UTC time of the fix */
    nmea_float_t latitude;  /**< Latitude in decimal degrees */
    nmea_float_t longitude; /**< Longitude in decimal degrees */
    uint16_t heading_ddeg;  /**< Heading in 0.1 degrees unit */
    int32_t altitude_dm;    /**< Altitude above MSL in dm */
    int32_t speed_dms;      /**< Ground speed in dm/s */
    int32_t hdop_d;         /**< Horizontal dilution of precision in 0.1 units, 1.0 to infinity */
    uint8_t satellites;     /**< Amount of used satellites */
    nmea_fix_quality_t fix_quality; /**< Quality of the current GPS fix */
    bool is_3d_fix;                 /**< Enough satellites for valid 3D fix obtained */
} gps_info_t;

typedef struct {
    uint8_t visible; /**< Total number of satellites in view */
    uint8_t count;   /**< Amount of valid records in sat array */
    nmea_sv_info_t sat[10];
} gps_sat_t;

/** GPS device description */
typedef struct {
    uint8_t uart_device; /**< UART device to use for GPS connection */
    ring_t ringbuf;      /**< Ringbuffer to store received data */
    char buf[32];        /**< Data storage for ringbuffer */

    /** GPS data are valid when set to 0x03, 0 invalid, 1 gga, 2 rmc */
    uint8_t data_valid;
    gps_info_t info;
    gps_sat_t sat;
} gps_desc_t;

/**
 * Put GPS device into standby mode (UART still active, GPS down)
 *
 * Should be called after the GPS is booted, else it won't work
 *
 * @param desc        Device descriptor
 */
void Gps_Standby(gps_desc_t *desc);

/**
 * Put GPS device into backup mode (needs restart or FORCE_UP signal to wake up)
 *
 * Should be called after the GPS is booted, else it won't work
 *
 * @param desc        Device descriptor
 */
void Gps_Backup(gps_desc_t *desc);

/**
 * Wake up GPS device from standby
 *
 * @param desc        Device descriptor
 */
void Gps_WakeUp(const gps_desc_t *desc);

/**
 * Get GPS data
 *
 * @param desc        Device descriptor
 * @return Data if any or NULL if no data received yet
 */
const gps_info_t *Gps_Get(gps_desc_t *desc);

/**
 * Get GPS satellite info
 *
 * @param desc        Device descriptor
 * @return Pointer to satellite data
 */
const gps_sat_t *Gps_GetSat(gps_desc_t *desc);

/**
 * Reset GPS recorded data to wait for brand new in loop/get functions
 *
 * @param desc        Device descriptor
 */
void Gps_InvalidateData(gps_desc_t *desc);

/**
 * Process received GPS data
 *
 * Should be called periodically as it fetches the data from uart buffer
 *
 * @param desc        Device descriptor
 * @return data If just received a position update
 */
const gps_info_t *Gps_Loop(gps_desc_t *desc);

/**
 * Initialize GPS module
 *
 * Due to implementation limitations, only one device can be used (
 * the receiving function is using descriptor provided in last Gps_Init call)
 *
 * @param [out] desc        Device descriptor
 * @param uart_device       UART device on which the GPS is connected
 */
void Gps_Init(gps_desc_t *desc, uint8_t uart_device);

#endif
