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
    if (value >= 0xF000) {
        return 0x3fff;
    } else if (value >= 0x7000) {
        return 0x3000 | ((value - 0x7000) >> 3);
    } else if (value >= 0x3000) {
        return 0x2000 | ((value - 0x3000) >> 2);
    } else if (value >= 0x1000) {
        return 0x1000 | ((value - 0x1000) >> 1);
    }
    return value;
}

uint16_t decodeFromUint14(uint16_t value)
{
    static const uint16_t offset[] = { 0, 0x1001, 0x3002, 0x7004 };
    uint8_t range = value >> 12;
    value &= 0x0FFF;

    return offset[range] + (value << range);
}

uint16_t encodeToUint10(uint16_t value)
{
    if (value >= 0xf00) {
        return 0x3ff;
    } else if (value >= 0x700) {
        return 0x300 | ((value - 0x700) >> 3);
    } else if (value >= 0x300) {
        return 0x200 | ((value - 0x300) >> 2);
    } else if (value >= 0x100) {
        return 0x100 | ((value - 0x100) >> 1);
    }
    return value;
}

uint16_t decodeFromUint10(uint16_t value)
{
    static const uint16_t offset[] = { 0, 0x101, 0x302, 0x704 };
    uint8_t range = value >> 8;
    value &= 0x00ff;

    return offset[range] + (value << range);
}

uint8_t encodeToUint6(uint8_t value)
{
    if (value >= 0xf0) {
        return 0x3f;
    } else if (value >= 0x70) {
        return 0x30 | ((value - 0x70) >> 3);
    } else if (value >= 0x30) {
        return 0x20 | ((value - 0x30) >> 2);
    } else if (value >= 0x10) {
        return 0x10 | ((value - 0x10) >> 1);
    }
    return value;
}

uint8_t decodeFromUint6(uint8_t value)
{
    static const uint8_t offset[] = { 0, 0x11, 0x32, 0x74 };
    uint8_t range = value >> 4;
    value &= 0x0f;

    return offset[range] + (value << range);
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

int16_t decodeSignedFromUint8(uint8_t value)
{
    static const uint8_t offset[] = { 0, 0x0021, 0x0062, 0x00e4 };
    int16_t sign = value & 0x80 ? -1 : 1;
    uint8_t range = (value & 0x7f) >> 5;
    value &= 0x1f;

    return (offset[range] + (value << range)) * sign;
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

int16_t decodeSignedFromUint9(uint16_t value)
{
    static const uint16_t offset[] = { 0, 0x0041, 0x00c2, 0x01c4 };
    int16_t sign = value & 0x100 ? -1 : 1;
    uint8_t range = (value & 0x00ff) >> 6;
    value &= 0x3f;

    return (offset[range] + (value << range)) * sign;
}
