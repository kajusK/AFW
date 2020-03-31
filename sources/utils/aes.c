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
 * @file    utils/aes.c
 * @brief   AES encryption
 *
 * The algorithm is described at wiki
 * https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
 *
 * Using a more computational heavy approach, but only 256 bytes of lookup
 * tables are required. AES steps for each round can be joined together using
 * bigger (4k) lookup tables. S_Box tables can be computed on the fly
 * to reduce memory footprint, but increases compute time.
 *
 * @addtogroup utils
 * @{
 */

#include <string.h>
#include "utils/aes.h"

/**
 * Lookup table for Forward S-box (https://en.wikipedia.org/wiki/Rijndael_S-box)
 */
static const uint8_t AESi_S_Box[] = {
    0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
    0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
    0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
    0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
    0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
    0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
    0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
    0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
    0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
    0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
    0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
    0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
    0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
    0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
    0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
    0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

/**
 * Lookup table for Inverse S-box (https://en.wikipedia.org/wiki/Rijndael_S-box)
 */
static const uint8_t AESi_S_Box_Inv[] = {
    0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF, 0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB,
    0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34, 0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB,
    0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE, 0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E,
    0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76, 0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25,
    0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4, 0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92,
    0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E, 0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84,
    0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7, 0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06,
    0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1, 0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B,
    0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97, 0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73,
    0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2, 0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E,
    0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F, 0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B,
    0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A, 0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4,
    0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F,
    0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D, 0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF,
    0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8, 0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D
};

/**
 * Multiplication by 2 in Rijndael's Galois field
 *
 * @param num       Number to be multiplied
 * @return num * 2 in finite field
 */
static uint8_t AESi_Mul2(uint8_t num)
{
    uint8_t reduce[] = {0, 0x1b};

    /* Just an attempt to make the timing constant */
    return (num << 1) ^ reduce[num >= 0x80];
}

/**
 * Division by 2 in Rijndael's Galois field
 *
 * @param num       Number to be divided
 * @return num/2 in finite field
 */
static uint8_t AESi_Div2(uint8_t num)
{
    uint8_t reduce[] = {0, 0x8d};

    /* Just an attempt to make the timing constant */
    return (num >> 1) ^ reduce[num & 0x01];
}

/**
 * Combine state with round key
 *
 * @param state     State to be updated
 * @param key       16 bytes of round key
 */
static void AESi_AddRoundKey(uint8_t *state, const uint8_t *key)
{
    uint8_t *end = state + 16;
    while (state != end) {
        *state++ ^= *key++;
    }
}

/**
 * Replace each byte of the state by corresponding value from S_Box lookup table
 *
 * @param state     State to be updated
 */
static void AESi_SubBytes(uint8_t *state)
{
    uint8_t *end = state + 16;
    while (state != end) {
        *state = AESi_S_Box[*state];
        state++;
    }
}

/**
 * Inverse operation to SubBytes
 *
 * @param state     State to be updated
 */
static void AESi_SubBytesInv(uint8_t *state)
{
    uint8_t *end = state + 16;
    while (state != end) {
        *state = AESi_S_Box_Inv[*state];
        state++;
    }
}

/**
 * Rotate state rows
 *
 * @param state     State to be updated
 */
static void AESi_ShiftRows(uint8_t *state)
{
    uint8_t tmp;

    /* First row not modified */
    /* Second row rotated by 1 to left */
    tmp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = tmp;

    /* Third row rotated by 2 to left */
    tmp = state[2];
    state[2] = state[10];
    state[10] = tmp;
    tmp = state[6];
    state[6] = state[14];
    state[14] = tmp;

    /* Fourth row rotated by 3 to left */
    tmp = state[3];
    state[3] = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = tmp;
}

/**
 * Rotate state rows in inverse direction
 *
 * @param state     State to be updated
 */
static void AESi_ShiftRowsInv(uint8_t *state)
{
    uint8_t tmp;

    /* First row not modified */
    /* Second row rotated by 1 to right */
    tmp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = tmp;

    /* Third row rotated by 2 to right */
    tmp = state[10];
    state[10] = state[2];
    state[2] = tmp;
    tmp = state[14];
    state[14] = state[6];
    state[6] = tmp;

    /* Fourth row rotated by 3 to right */
    tmp = state[3];
    state[3] = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = tmp;
}

/**
 * Combine 4 bytes in each column
 *
 * Algorith from https://en.wikipedia.org/wiki/Rijndael_MixColumns
 *
 * @param state     State to be updated
 */
static void AESi_MixColumns(uint8_t *state)
{
    uint8_t a, a2;
    uint8_t buf[4];
    uint8_t *end = state+16;

    while (state != end) {
        memset(buf, 0x00, 4);
        for (uint8_t i = 0; i < 4; i++) {
            /* a is copy of state, a2 is a*2, a^a2 is a*3 */
            a = state[i];
            a2 = AESi_Mul2(state[i]);
            buf[i] ^= a2;
            buf[(i+1)%4] ^= a;
            buf[(i+2)%4] ^= a;
            buf[(i+3)%4] ^= a2 ^ a;
        }
        memcpy(state, buf, 4);
        state += 4;
    }
}

/**
 * Inverse function to MixColumns
 *
 * Algorith from https://en.wikipedia.org/wiki/Rijndael_MixColumns
 *
 * @param state     State to be updated
 */
static void AESi_MixColumnsInv(uint8_t *state)
{
    uint8_t a, a2, a4, a8;
    uint8_t buf[4];
    uint8_t *end = state+16;

    while (state != end) {
        memset(buf, 0x00, 4);
        for (uint8_t i = 0; i < 4; i++) {
            a = state[i];
            a2 = AESi_Mul2(a);
            a4 = AESi_Mul2(a2);
            a8 = AESi_Mul2(a4);
            buf[i] ^= a8 ^ a4 ^ a2;
            buf[(i+1)%4] ^= a8 ^ a;
            buf[(i+2)%4] ^= a8 ^ a4 ^ a;
            buf[(i+3)%4] ^= a8 ^ a2 ^ a;
        }
        memcpy(state, buf, 4);
        state += 4;
    }
}

/**
 * Calculate new round key (another 16 bytes of the round key per round)
 *
 * https://en.wikipedia.org/wiki/AES_key_schedule
 *
 * @param key       Key to be recalculated
 * @param round     Rcon for current round
 */
static void AESi_GenRoundKey(uint8_t *key, uint8_t rcon)
{
    uint8_t *end = key + 12;

    /* rotate last column to left, apply sbox */
    key[0] ^= AESi_S_Box[key[13]] ^ rcon;
    key[1] ^= AESi_S_Box[key[14]];
    key[2] ^= AESi_S_Box[key[15]];
    key[3] ^= AESi_S_Box[key[12]];

    while (key < end) {
        key[4] ^= key[0];
        key[5] ^= key[1];
        key[6] ^= key[2];
        key[7] ^= key[3];
        key += 4;
    }
}

/**
 * Calculate new round key in inverse direction
 *
 * @param key       Key to be recalculated
 * @param round     Rcon for current round
 */
static void AESi_GenRoundKeyInv(uint8_t *key, uint8_t rcon)
{
    uint8_t *pos = key + 8;

    while (pos >= key) {
        pos[4] ^= pos[0];
        pos[5] ^= pos[1];
        pos[6] ^= pos[2];
        pos[7] ^= pos[3];
        pos -= 4;
    }

    /* rotate last column to left, apply sbox */
    key[0] ^= AESi_S_Box[key[13]] ^ rcon;
    key[1] ^= AESi_S_Box[key[14]];
    key[2] ^= AESi_S_Box[key[15]];
    key[3] ^= AESi_S_Box[key[12]];
}

/**
 * Generate initial round key for decryption (last round key from encryption)
 *
 * @param key       AES128 key, result will be stored here
 */
static void AESi_GenRoundKeyLast(uint8_t *key)
{
    uint8_t rcon = 1;
    for (uint8_t i = 0; i < 10; i++) {
        AESi_GenRoundKey(key, rcon);
        rcon = AESi_Mul2(rcon);
    }
}

/**
 * Shift all bits in a key to the left by 1
 *
 * @param key       16 bytes Key to be modified, result is stored here
 */
static void AESi_ShiftKeyLeft(uint8_t *key)
{
    uint8_t overflow = 0;

    for (uint8_t i = 0; i < 15; i++) {
        overflow = key[i+1] & 0x80 ? 1 : 0;
        key[i] = (key[i] << 1) + overflow;
    }
    key[15] <<= 1;
}

void AES128_Encrypt(uint8_t *data, const uint8_t *key)
{
    uint8_t *state = data;
    uint8_t roundKey[16];
    uint8_t i;
    uint8_t rcon = 1;

    memcpy(roundKey, key, 16);
    AESi_AddRoundKey(state, roundKey);

    for (i = 0; i < 9; i++) {
        AESi_SubBytes(state);
        AESi_ShiftRows(state);
        AESi_MixColumns(state);

        /* Get new round key */
        AESi_GenRoundKey(roundKey, rcon);
        AESi_AddRoundKey(state, roundKey);
        rcon = AESi_Mul2(rcon);
    }

    AESi_SubBytes(state);
    AESi_ShiftRows(state);
    AESi_GenRoundKey(roundKey, rcon);
    AESi_AddRoundKey(state, roundKey);
}

void AES128_Decrypt(uint8_t *data, const uint8_t *key)
{
    uint8_t *state = data;
    uint8_t roundKey[16];
    uint8_t rcon = 0x36; /* last rcon value */

    memcpy(roundKey, key, 16);
    AESi_GenRoundKeyLast(roundKey);

    AESi_AddRoundKey(state, roundKey);
    AESi_GenRoundKeyInv(roundKey, rcon);
    AESi_ShiftRowsInv(state);
    AESi_SubBytesInv(state);

    for (uint8_t i = 9; i != 0; i--) {
        rcon = AESi_Div2(rcon);
        AESi_AddRoundKey(state, roundKey);
        AESi_GenRoundKeyInv(roundKey, rcon);
        AESi_MixColumnsInv(state);
        AESi_ShiftRowsInv(state);
        AESi_SubBytesInv(state);
    }

    AESi_AddRoundKey(state, roundKey);
}

void AES128_CMACGetKeys(uint8_t *k1, uint8_t *k2, const uint8_t *key)
{
    uint8_t xor = 0;
    memset(k1, 0x00, 16);
    memset(k2, 0x00, 16);

    AES128_Encrypt(k1, key);
    if (k1[0] & 0x80) {
        xor = 0x87;
    }
    AESi_ShiftKeyLeft(k1);
    k1[15] ^= xor;

    memcpy(k2, k1, 16);
    AESi_ShiftKeyLeft(k2);
    if (k1[0] & 0x80) {
        k2[15] ^= 0x87;
    }
}

void AES128_CMAC(const uint8_t *data, size_t len, const uint8_t *key,
        uint8_t *tag)
{
    uint8_t k1[16];
    uint8_t k2[16];
    uint8_t i;

    /* Prepare keys */
    AES128_CMACGetKeys(k1, k2, key);

    /* Run the cmac algorithm */
    memset(tag, 0, 16);
    /* First process n-1 blocks */
    while (len > 16) {
        for (i = 0; i < 16; i++) {
            tag[i] ^= *data++;
        }
        AES128_Encrypt(tag, key);
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
    AES128_Encrypt(tag, key);
}

/** @} */
