/*
 * Copyright (C) 2020 Jakub Kaderka
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
 * @file    modules/ttn.c
 * @brief   LoRaWan MAC layer implementation (for TTN and other networks)
 *
 * Based on LoRaWan v1.1 specs
 * https://lora-alliance.org/sites/default/files/2018-04/lorawantm_specification_-v1.1.pdf
 *
 * Currently only partial mode A support is implemented (transmit only)
 *
 * @addtogroup modules
 * @{
 */

#include <string.h>
#include <modules/log.h>
#include <utils/aes.h>
#include <utils/assert.h>

#include "modules/lora.h"

/** Transmitted frame counter */
static uint32_t lorai_frame_tx_cnt;
/** Device address, 4 bytes */
static const uint8_t *lorai_DevAddr;
/** Network Session key, 16 bytes */
static const uint8_t *lorai_NwkSkey;
/** Application Session key, 16 bytes */
static const uint8_t *lorai_AppSkey;
/** Callback used to send data - tied to RF driver */
static lora_send_cb_t lorai_send_cb;

/**
 * Encrypt the LoRa payload using the AppSKey (for ports 1-255)
 *
 * @param data  Data to be encrypted, result is stored here
 * @param len   Length of the data to be encrypted
 * @param frame_cnt     Frame counter to be used
 * @param tx            True if encryption outgoing transmission (uplink)
 */
static void Lorai_PayloadEncrypt(uint8_t *data, uint8_t len, uint32_t frame_cnt,
        bool tx)
{
    uint8_t block[16];
    uint8_t i, j;
    /* apply ceil to len / 16 to get number of blocks required */
    uint8_t k = (len + 15) / 16;

    ASSERT_NOT(lorai_AppSkey == NULL);

    for (i = 1; i <= k; i++) {
        /* Lora encryption schema is strange... */
        block[0] = 0x01;
        block[1] = 0x00;
        block[2] = 0x00;
        block[3] = 0x00;
        block[4] = 0x00;
        block[5] = tx == 0 ? 0x01 : 0x00;
        block[6] = lorai_DevAddr[3];
        block[7] = lorai_DevAddr[2];
        block[8] = lorai_DevAddr[1];
        block[9] = lorai_DevAddr[0];
        block[10] = frame_cnt & 0xff;
        block[11] = (frame_cnt >> 8) & 0xff;
        block[12] = (frame_cnt >> 16) & 0xff;
        block[13] = (frame_cnt >> 24) & 0xff;
        block[14] = 0x00;
        block[15] = i;

        AES128_Encrypt(block, lorai_AppSkey);

        for (j = 0; j < 16 && j < len; j++) {
            *data ^= block[j];
            data++;
        }
        len -= j;
    }
}

/**
 * Calculate MIC block for given payload
 *
 * @param mic   4 bytes where mic should be stored
 * @param data  Data buffer to calculate MIC for
 * @param len   Length of the buffer
 * @param frame_cnt Frame counter to be used
 * @param tx            True if encryption outgoing transmission (uplink)
 */
static void Lorai_GetMIC(uint8_t *mic, uint8_t *data, uint8_t len,
        uint32_t frame_cnt, bool tx)
{
    uint8_t k1[16], k2[16];
    uint8_t tag[16];
    uint8_t i;

    /*
     * Lora encryption schema is strange, AES128_CMAC is run on payload with
     * following data prepended
     */
    tag[0] = 0x49;
    tag[1] = 0x00;
    tag[2] = 0x00;
    tag[3] = 0x00;
    tag[4] = 0x00;
    tag[5] = tx == 0 ? 0x01 : 0x00;
    tag[6] = lorai_DevAddr[3];
    tag[7] = lorai_DevAddr[2];
    tag[8] = lorai_DevAddr[1];
    tag[9] = lorai_DevAddr[0];
    tag[10] = frame_cnt & 0xff;
    tag[11] = (frame_cnt >> 8) & 0xff;
    tag[12] = (frame_cnt >> 16) & 0xff;
    tag[13] = (frame_cnt >> 24) & 0xff;
    tag[14] = 0x00;
    tag[15] = len;

    /* Run the cmac algorithm */
    AES128_CMACGetKeys(k1, k2, lorai_NwkSkey);
    AES128_Encrypt(tag, lorai_NwkSkey);

    /* First process n-1 blocks */
    while (len > 16) {
        for (i = 0; i < 16; i++) {
            tag[i] ^= *data++;
        }
        AES128_Encrypt(tag, lorai_NwkSkey);
        len -= 16;
    }

    if (len == 16) {
        /* complete block */
        for (i = 0; i < len; i++) {
            tag[i] ^= (*data++) ^ k1[i];
        }
    } else {
        /* incomplete block */
        for (i = 0; i < len; i++) {
            tag[i] ^= (*data++) ^ k2[i];
        }
        tag[i] ^= 0x80 ^ k2[i];
        for (i += 1; i < 16; i++) {
            tag[i] ^= k2[i];
        }
    }
    AES128_Encrypt(tag, lorai_NwkSkey);

    memcpy(mic, tag, 4);
}

bool Lora_Send(const uint8_t *data, uint8_t len)
{
    uint8_t message[LORA_MAX_PAYLOAD_LEN + 13];
    ASSERT_NOT(lorai_send_cb == NULL || lorai_DevAddr == NULL);

    /*
     * The session keys are supplied by init function for ABP or by the join
     * request during otaa
     */
    if (lorai_NwkSkey == NULL || lorai_AppSkey == NULL) {
        return false;
    }

    /* MAC header - unconfirmed data up, LoRaWANR1 */
    message[0] = 0x40;
    /* Device address */
    message[1] = lorai_DevAddr[3];
    message[2] = lorai_DevAddr[2];
    message[3] = lorai_DevAddr[1];
    message[4] = lorai_DevAddr[0];
    /* Frame control */
    message[5] = 0x00;
    /* Frame counter */
    message[6] = lorai_frame_tx_cnt & 0x00ff;
    message[7] = (lorai_frame_tx_cnt >> 8) & 0x00ff;
    /* Frame port, 0 for MAC commands, 1-223 for application data */
    message[8] = 1; /* let's use first available for now */

    memcpy(&message[9], data, len);
    Lorai_PayloadEncrypt(&message[9], len, lorai_frame_tx_cnt, true);
    Lorai_GetMIC(&message[9+len], message, len+9, lorai_frame_tx_cnt, true);

    lorai_send_cb(message, len + 13);
    lorai_frame_tx_cnt++;

    return true;
}

void Lora_ResetFrameCounters(void)
{
    lorai_frame_tx_cnt = 0;
}

void Lora_SetCounters(uint32_t frame_rx, uint32_t frame_tx)
{
    (void) frame_rx;
    lorai_frame_tx_cnt = frame_tx;
}

void Lora_GetCounters(uint32_t *frame_rx, uint32_t *frame_tx)
{
    if (frame_rx != NULL) {
        *frame_rx = 0;
    }
    if (frame_tx != NULL) {
        *frame_tx = lorai_frame_tx_cnt;
    }
}

void Lora_InitAbp(lora_send_cb_t send, const uint8_t *DevAddr,
        const uint8_t *NwkSkey, const uint8_t *AppSkey)
{
    lorai_send_cb = send;
    lorai_NwkSkey = NwkSkey;
    lorai_AppSkey = AppSkey;
    lorai_DevAddr = DevAddr;
    Lora_ResetFrameCounters();
}

/** @} */
