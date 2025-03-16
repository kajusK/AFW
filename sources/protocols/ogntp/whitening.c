/**
 * @file    ldpc.c
 * @brief   Low-Density parity-check code for OGNTP frames
 */

#include <types.h>
#include <string.h>
#include "ogn_internal.h"

#define DELTA 0x9e3779b9

void whitenPayload(uint8_t payload[16])
{
    uint32_t data[4];
    uint32_t sum = 0;

    memcpy(data, payload, sizeof(data)); // avoid unaligned memory access
    for (uint8_t i = 0; i < 8; i++) {
        sum += DELTA;
        data[0] += (data[1] << 4) ^ (data[1] + sum) ^ (data[1] >> 5);
        data[1] += (data[0] << 4) ^ (data[0] + sum) ^ (data[0] >> 5);
        data[2] += (data[3] << 4) ^ (data[3] + sum) ^ (data[3] >> 5);
        data[3] += (data[2] << 4) ^ (data[2] + sum) ^ (data[2] >> 5);
    }
    memcpy(payload, data, sizeof(data));
}

void dewhitenPayload(uint8_t payload[16])
{
    uint32_t data[4];
    uint32_t sum = DELTA * 8;

    memcpy(data, payload, sizeof(data));
    for (uint8_t i = 0; i < 8; i++) {
        data[1] -= (data[0] << 4) ^ (data[0] + sum) ^ (data[0] >> 5);
        data[0] -= (data[1] << 4) ^ (data[1] + sum) ^ (data[1] >> 5);
        data[3] -= (data[2] << 4) ^ (data[2] + sum) ^ (data[2] >> 5);
        data[2] -= (data[3] << 4) ^ (data[3] + sum) ^ (data[3] >> 5);
        sum -= DELTA;
    }
    memcpy(payload, data, sizeof(data));
}
