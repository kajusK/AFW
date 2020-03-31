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
 * @file    modules/lora.h
 * @brief   LoRaWan MAC layer implementation (for TTN and other networks)
 *
 * @addtogroup modules
 * @{
 */

#ifndef __MODULES_LORA_H
#define __MODULES_LORA_H

#include <types.h>

/**
 * Maximum size of the lora message payload (actual value should follow
 * the lora regional constraints)
 */
#define LORA_MAX_PAYLOAD_LEN 64

/**
 * Callback used to send lora packet over RF
 *
 * @param data      Data to be send
 * @param len       Lenght of the data
 */
typedef void (*lora_send_cb_t)(const uint8_t *data, size_t len);

/**
 * Send data to LoRaWan gateway
 *
 * @param data      Data to be transmitted
 * @param len       Length of the data (up to LORA_MAX_PAYLOAD_LEN)
 *                  it should be region and datarate specific, check specs
 *
 * @return True if succeeded, false otherwise (session keys not set)
 *
 * @TODO block until second receive window end (node should not transmit sooner)
 */
extern bool Lora_Send(const uint8_t *data, uint8_t len);

/**
 * Reset LoRaWan frame counters back to 0
 */
extern void Lora_ResetFrameCounters(void);

/**
 * Set internal frame counters (e.g. after reboot)
 *
 * @param frame_rx  Received frames counter
 * @param frame_tx  Sent frame sounter
 */
extern void Lora_SetCounters(uint32_t frame_rx, uint32_t frame_tx);

/**
 * Get internal frame counters (e.g. to save before reboot when in ABP mode)
 *
 * @param frame_rx  Received frames counter (or NULL if not needed)
 * @param frame_tx  Sent frame sounter (or NULL if not needed)
 */
extern void Lora_GetCounters(uint32_t *frame_rx, uint32_t *frame_tx);

/**
 * Initialize LoRaWan module in Activation by Personalization (ABP)
 *
 * CAUTION - all memory addresses (device key,...) supplied to the function must
 * remain accessible during the runtime, the library stores only pointer to
 * these values!
 *
 * @param send      Callback used to send assembled lora packet
 * @param DevAddr   4 bytes of the Device Address
 * @param NwkSkey   16 bytes of the Network Session Key
 * @param AppSkey   16 bytes of the Application Session Key
 */
extern void Lora_InitAbp(lora_send_cb_t send, const uint8_t *DevAddr,
        const uint8_t *NwkSkey, const uint8_t *AppSkey);

#endif

/** @} */
