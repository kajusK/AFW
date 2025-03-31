/**
 * @file    encoding.h
 * @brief   OGN Tracking Protocol binary encoding implementation
 *
 * OGNTP encodes/compresses binary values in a loose format where higher
 * values are encoded with a lower precision. Negative values are marked
 * have a sign bit, where negative zero equals to value not available/invalid.
 */

#ifndef _OGNTP_INTERNAL_H_
#define _OGNTP_INTERNAL_H_

#include <types.h>

/** Calculate LDPC parity bits for OGNTP frame - 48 parity bits for 160 data bits */
void getFCS(const uint8_t data[20], uint8_t parity[6]);

/** Verify the frame validity by the parity bits */
bool isFCSValid(const uint8_t data[20], const uint8_t expected_parity[6]);

/** Whiten the frame payload (without header), based on TEA encryption with zero key */
void whitenPayload(uint8_t payload[16]);

/** De-Whiten the data payload (without header) */
void dewhitenPayload(uint8_t payload[16]);

/** Encode unsigned number (0 to 61440) to 14 bits */
uint16_t encodeToUint14(uint16_t value);
uint16_t decodeFromUint14(uint16_t value);

/* Encode unsigned number (0 to 3840) to 10 bits with compression */
uint16_t encodeToUint10(uint16_t value);
uint16_t decodeFromUint10(uint16_t value);

/** Encode unsigned number (0 to 240) to 4 bits with compression */
uint8_t encodeToUint6(uint8_t value);
uint8_t decodeFromUint6(uint8_t value);

/** Encode signed number (-480 to 480) to 8 bit with compression */
uint8_t encodeSignedToUint8(int16_t value);
int16_t decodeSignedFromUint8(uint8_t value);

/** Encode signed number (-960 to 960) to 9 bits with compression */
uint16_t encodeSignedToUint9(int16_t value);
int16_t decodeSignedFromUint9(uint16_t value);

#endif
