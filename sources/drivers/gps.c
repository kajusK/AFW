/**
 * @file    drivers/gps.h
 * @brief   Driver for gps receiver (sleep/wake up implemented for SIM28)
 *
 * For PMTK commands, see
 * https://www.rhydolabz.com/documents/25/PMTK_A11.pdf
 */

#include <types.h>
#include "hal/uart.h"
#include "hal/io.h"
#include "utils/ringbuf.h"
#include "utils/time.h"
#include "modules/log.h"
#include "gps.h"

/** Data older than 5 seconds are considered invalid */
#define GPS_VALID_TIMEOUT_MS 5000

/** Descriptor used for receiving function */
static gps_desc_t *gpsi_desc;

/**
 * Callback for data received from GPS
 *
 * Process received NMEA message and store data to external structure
 */
static void Gpsi_RxCb(uint8_t c)
{
    if (gpsi_desc != NULL) {
        Ring_Push(&gpsi_desc->ringbuf, c);
    }
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
        return f->num * (scale / f->scale);
    }
    return f->num / (f->scale / scale);
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

    info->latitude = rmc.lat;
    info->longitude = rmc.lon;
    info->speed_dms = Gpsi_NmeaF2Dec(&rmc.speed_ms, 10);
    info->heading_ddeg = Gpsi_NmeaF2Dec(&rmc.heading, 10);
    info->time = rmc.fix_time;
    info->date = rmc.date;
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
    info->latitude = gga.lat;
    info->longitude = gga.lon;
    info->hdop_d = Gpsi_NmeaF2Dec(&gga.hdop, 10);
    info->altitude_dm = Gpsi_NmeaF2Dec(&gga.altitude_m, 10);
    info->fix_quality = gga.quality;
    info->is_3d_fix = gga.quality >= 1 && gga.satellites >= 4;
    return true;
}

static void Gpsi_ProcessGsv(const char *msg, gps_sat_t *info)
{
    nmea_gsv_t gsv;
    uint8_t pos;
    uint8_t count;

    if (!Nmea_ParseGsv(msg, &gsv)) {
        return;
    }

    info->visible = gsv.visible;

    for (int i = 0; i < gsv.count; i++) {
        /* There are up to 4 satellites per message */
        pos = 4 * (gsv.msg_id - 1) + i;
        if (pos >= sizeof(info->sat) / sizeof(info->sat[0])) {
            break;
        }

        info->sat[pos] = gsv.sv[i];
    }

    if (gsv.messages == gsv.msg_id) {
        count = (gsv.messages - 1) * 4 + gsv.count;
        if (count > sizeof(info->sat) / sizeof(info->sat[0])) {
            count = sizeof(info->sat) / sizeof(info->sat[0]);
        }
        info->count = count;
    }
}

void Gps_Standby(gps_desc_t *desc)
{
    desc->data_valid = 0;
    UARTd_Puts(desc->uart_device, "$PMTK161,0*28\r\n");
}

void Gps_Backup(gps_desc_t *desc)
{
    desc->data_valid = 0;
    UARTd_Puts(desc->uart_device, "$PMTK225,4*2F\r\n");
}

void Gps_WakeUp(const gps_desc_t *desc)
{
    /* Any data will wake the device up from standby mode */
    UARTd_Puts(desc->uart_device, "$PMTK000*32\r\n");
}

const gps_info_t *Gps_Get(gps_desc_t *desc)
{
    if (desc->data_valid == 0x03) {
        return &desc->info;
    }
    return NULL;
}

const gps_sat_t *Gps_GetSat(gps_desc_t *desc)
{
    return &desc->sat;
}

void Gps_InvalidateData(gps_desc_t *desc)
{
    desc->data_valid = 0;
}

const gps_info_t *Gps_Loop(gps_desc_t *desc)
{
    bool was_updated = false;

    while (!Ring_Empty(&desc->ringbuf)) {
        const char *msg = Nmea_AddChar(Ring_Pop(&desc->ringbuf));
        if (msg == NULL) {
            continue;
        }

        Log_Debug("GPS", msg);
        switch (Nmea_GetSentenceType(msg)) {
            case NMEA_SENTENCE_GGA:
                if (Gpsi_ProcessGga(msg, &desc->info)) {
                    desc->data_valid |= 0x01;
                    was_updated = true;
                }
                break;
            case NMEA_SENTENCE_RMC:
                if (Gpsi_ProcessRmc(msg, &desc->info)) {
                    desc->data_valid |= 0x02;
                    desc->info.timestamp = millis();
                    was_updated = true;
                }
                break;
            case NMEA_SENTENCE_GSV:
                Gpsi_ProcessGsv(msg, &desc->sat);
                break;
            default:
                break;
        }
    }

    /* Invalidate data if no update was received withing reasonable time */
    if (millis() - desc->info.timestamp > GPS_VALID_TIMEOUT_MS) {
        desc->data_valid = 0;
    }

    if (desc->data_valid == 0x03 && was_updated) {
        return &desc->info;
    }
    return NULL;
}

void Gps_Init(gps_desc_t *desc, uint8_t uart_device)
{
    ASSERT_NOT(desc == NULL);

    desc->uart_device = uart_device;
    Ring_Init(&desc->ringbuf, desc->buf, sizeof(desc->buf));
    gpsi_desc = desc;
    UARTd_SetRxCallback(uart_device, Gpsi_RxCb);
}
