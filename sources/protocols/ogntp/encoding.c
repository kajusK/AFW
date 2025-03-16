/**
 * @file    encoding.c
 * @brief   OGN Tracking Protocol binary encoding implementation
 *
 * OGNTP encodes/compresses binary values in a loose format where higher
 * values are encoded with a lower precision. All ones means overflow.
 * Negative values are marked have a sign bit, where negative zero equals to
 * value not available/invalid.
 */

#include <types.h>
#include "ogn_internal.h"

uint16_t encodeToUint14(uint16_t value)
{
    if (value < 0x1000) {
        // keep unchanged
    } else if (value < 0x3000) {
        value = 0x1000 | ((value - 0x1000) >> 1);
    } else if (value < 0x7000) {
        value = 0x2000 | ((value - 0x3000) >> 2);
    } else if (value < 0xF000) {
        value = 0x3000 | ((value - 0x7000) >> 3);
    } else {
        value = 0x3fff;
    }
    return value;
}

uint16_t encodeToUint10(uint16_t value)
{
    if (value < 0x100) {
        // keep unchanged
    } else if (value < 0x300) {
        value = 0x100 | ((value - 0x100) >> 1);
    } else if (value < 0x700) {
        value = 0x200 | ((value - 0x300) >> 2);
    } else if (value < 0xf00) {
        value = 0x300 | ((value - 0x700) >> 3);
    } else {
        value = 0x3ff;
    }
    return value;
}

uint8_t encodeToUint4(uint8_t value)
{
    if (value < 0x10) {
        // keep unchanged
    } else if (value < 0x30) {
        value = 0x10 | ((value - 0x10) >> 1);
    } else if (value < 0x70) {
        value = 0x20 | ((value - 0x30) >> 2);
    } else if (value < 0xF0) {
        value = 0x30 | ((value - 0x70) >> 3);
    } else {
        value = 0x3f;
    }
    return value;
}

uint8_t encodeSignedToUint8(int16_t value)
{
    uint8_t sign = 0;
    if (value < 0) {
        value = -value;
        sign = 0x80;
    }

    if (value < 0x020) {
        // unchanged
    } else if (value < 0x060) {
        value = 0x020 | ((value - 0x020) >> 1);
    } else if (value < 0x0e0) {
        value = 0x040 | ((value - 0x060) >> 2);
    } else if (value < 0x1e0) {
        value = 0x060 | ((value - 0x0e0) >> 3);
    } else {
        value = 0x07f;
    }
    return value | sign;
}

uint16_t encodeSignedToUint9(int16_t value)
{
    uint16_t sign = 0;
    if (value < 0) {
        value = -value;
        sign = 0x100;
    }

    if (value < 0x040) {
        // unchanged
    } else if (value < 0x0c0) {
        value = 0x040 | ((value - 0x040) >> 1);
    } else if (value < 0x1c0) {
        value = 0x080 | ((value - 0x0c0) >> 2);
    } else if (value < 0x3c0) {
        value = 0x0c0 | ((value - 0x1c0) >> 3);
    } else {
        value = 0x0ff;
    }
    return value | sign;
}
