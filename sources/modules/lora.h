/**
 * @file    modules/lora.h
 * @brief   LoRaWan MAC layer implementation (for TTN and other networks)
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
bool Lora_Send(const uint8_t *data, uint8_t len);

/**
 * Reset LoRaWan frame counters back to 0
 */
void Lora_ResetFrameCounters(void);

/**
 * Set internal frame counters (e.g. after reboot)
 *
 * @param frame_rx  Received frames counter
 * @param frame_tx  Sent frame sounter
 */
void Lora_SetCounters(uint32_t frame_rx, uint32_t frame_tx);

/**
 * Get internal frame counters (e.g. to save before reboot when in ABP mode)
 *
 * @param frame_rx  Received frames counter (or NULL if not needed)
 * @param frame_tx  Sent frame sounter (or NULL if not needed)
 */
void Lora_GetCounters(uint32_t *frame_rx, uint32_t *frame_tx);

/**
 * Set keys for Activation by Personalization (ABP)
 *
 * CAUTION - all memory addresses (device key,...) supplied to the function must
 * remain accessible during the runtime, the library stores only pointer to
 * these values!
 *
 * @param DevAddr   4 bytes of the Device Address
 * @param NwkSkey   16 bytes of the Network Session Key
 * @param AppSkey   16 bytes of the Application Session Key
 */
void Lora_SetAbpKeys(const uint8_t *DevAddr, const uint8_t *NwkSkey,
        const uint8_t *AppSkey);

/**
 * Initialize LoRaWan module in Activation by Personalization (ABP)
 *
 * @param send      Callback used to send assembled lora packet
 */
void Lora_InitAbp(lora_send_cb_t send);

#endif
