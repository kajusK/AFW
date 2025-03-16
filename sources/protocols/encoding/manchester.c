/**
 * @file    manchester.c
 * @brief   Manchester data encoding/decoding
 *
 * Manchester encoding converts 1 to 01 and 0 to 10 and vice versa.
 */

#include <types.h>

/** Lookup table for manchester encoding (4bit input) */
static const uint8_t encodeTable[] = {
    0xAA,
    0xA9,
    0xA6,
    0xA5,
    0x9A,
    0x99,
    0x96,
    0x95,
    0x6A,
    0x69,
    0x66,
    0x65,
    0x5A,
    0x59,
    0x56,
    0x55,
};

void ManchesterEncode(uint8_t *output, uint8_t *input, uint32_t len)
{
    while (len-- > 0) {
        *output++ = encodeTable[(*input >> 4) & 0x0f];
        *output++ = encodeTable[*input & 0x0f];
        input++;
    }
}

bool ManchesterDecode(uint8_t *output, uint8_t *input, uint32_t len)
{
    if ((len & 0x01) != 0) {
        return false;
    }

    for (uint32_t i = 0; i < len; i += 2) {
        uint16_t data = input[i + 1] | ((uint16_t)input[i] << 8);
        *output = 0;

        for (uint8_t j = 0; j < 8; j++) {
            *output >>= 1;
            if ((data & 0x03) == 0x01) {
                *output |= 0x80;
            } else if ((data & 0x03) != 0x02) {
                return false;
            }
            data >>= 2;
        }
        output++;
    }
    return true;
}
