/**
 * @file    manchester.h
 * @brief   Manchester data encoding/decoding
 *
 * Manchester encoding converts 1 to 01 and 0 to 10 and vice versa.
 */

#ifndef _MANCHESTER_H_
#define _MANCHESTER_H_

#include <types.h>

/**
 * Encode data with manchester encoding
 *
 * @param output    Output buffer, 2x length of the input
 * @param input     Data to be encoded
 * @param len       Length of the input data
 */
void ManchesterEncode(uint8_t *output, const uint8_t *input, uint32_t len);

/**
 * Decode manchester encoded data
 *
 * @param output    Output buffer, 1/2 length of the input
 * @param input     Data to be decoded, length must be multiple of 2 bytes
 * @param len       Length of the input data
 * @return True if ok, false if data are invalid
 */
bool ManchesterDecode(uint8_t *output, const uint8_t *input, uint32_t len);

#endif
