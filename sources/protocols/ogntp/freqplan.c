/**
 * @file    freqplan.c
 * @brief   OGN Tracking Protocol frequency plan and slot timing
 */

#include "utils/nav.h"
#include "ogntp.h"

typedef struct {
    uint32_t start_freq_hz;    /**< Channel 0 frequency */
    uint32_t ch_separation_hz; /**< Distance between channel */
    uint32_t channels;         /**< Total amount of channels */
} freq_plan_t;

static const freq_plan_t *getFreqPlan(nav_region_t region)
{
    static const freq_plan_t europe = { 868200000, 200000, 2 };
    static const freq_plan_t north_america = { 902200000, 400000, 65 };
    static const freq_plan_t australia = { 917000000, 400000, 24 };

    switch (region) {
        /* EU 863 to 870 MHz band */
        case NAV_REGION_EUROPE:
        case NAV_REGION_AFRICA:
            return &europe;

        /* US 902 to 928 MHz band */
        case NAV_REGION_NORTH_AMERICA:
            return &north_america;

        /* Split between AU915-928 and US902-928 bands */
        case NAV_REGION_SOUTH_AMERICA:
        case NAV_REGION_AUSTRALIA_ZEELAND:
            return &australia;

        /* Mess, many different bands split by countries, disabled for now */
        case NAV_REGION_ASIA:
        default:
            return NULL;
    }
}

static uint32_t getFreqHz(const freq_plan_t *plan, uint32_t channel)
{
    return plan->start_freq_hz + plan->ch_separation_hz * channel;
}

/**
 * FLARM time hashing function for frequency hopping
 *
 * @param timestamp UTC UNIX timestamp for frequency hoping algorithm
 */
static uint32_t getHopHash(uint32_t timestamp)
{
    timestamp = (timestamp << 15) + (~timestamp);
    timestamp ^= timestamp >> 12;
    timestamp += timestamp << 2;
    timestamp ^= timestamp >> 4;
    timestamp *= 2057;
    timestamp ^= (timestamp >> 16);
    return timestamp;
}

/**
 * Get channel for given time slot
 *
 * @note Based on FLARM frequency hoping, just moved a bit to avoid collisions
 *
 * @param plan      Currently used frequency plan
 * @param slot      Timeslot, 0 or 1
 * @param timestamp UTC UNIX timestamp for frequency hoping algorithm
 * @return channel ID
 */
static uint32_t getChannel(const freq_plan_t *plan, uint8_t slot, uint32_t timestamp)
{
    uint32_t channel = 0;

    if (plan->channels <= 1) {
        return 0;
    }
    if (plan->channels == 2) {
        return slot ^ 0x01; // reverse to FLARM
    }

    // Start with same channel as FLARM
    channel = getHopHash((timestamp << 1) + slot) % plan->channels;
    if (slot == 0) {
        channel++; // increase to avoid collision with FLARM
    } else {
        // get channel for FLARM slot 0 / OGN slot 1
        uint8_t channel2 = getHopHash(timestamp << 1) % plan->channels;

        if (channel2 != channel) {
            channel = channel2; // no collision with FLARM, use this channel
        } else {
            channel++; // increase to avoid collision with FLARM
        }
    }

    if (channel >= plan->channels) {
        channel -= 2; // -2 to avoid FLARM collision (-1 in FLARM?)
    }
    return channel;
}

uint32_t OGNTP_GetFrequencyHz(nav_region_t region, uint8_t slot, uint32_t timestamp_ms)
{
    const freq_plan_t *plan = getFreqPlan(region);
    if (plan == NULL || slot > 1) {
        return 0;
    }
    uint32_t channel = getChannel(plan, slot, timestamp_ms);
    return getFreqHz(plan, channel);
}
