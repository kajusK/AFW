/**
 * @file    utils/aes.h
 * @brief   AES encryption
 *
 * WARNING - this library was written without side channels attacks in mind,
 * it can be attacked easily by timing attacks, etc.
 */

#ifndef __UTILS_AES_H
#define __UTILS_AES_H

#include <types.h>

/**
 * Encrypt 16 bytes of data by AES128
 *
 * @param data  16 bytes to encrypt, result is stored here
 * @param key   128bit encryption key
 */
void AES128_Encrypt(uint8_t *data, const uint8_t *key);

/**
 * Encrypt 16 bytes of data by AES128
 *
 * @param data  16 bytes to decrypt, result is stored here
 * @param key   128bit encryption key
 */
void AES128_Decrypt(uint8_t *data, const uint8_t *key);

/**
 * Generate k1 and k2 keys for AES128 CMAC computation
 *
 * @param k1        16 bytes buffer for k1
 * @param k2        16 bytes buffer for k2
 * @param key       128bit encryption key
 */
void AES128_CMACGetKeys(uint8_t *k1, uint8_t *k2, const uint8_t *key);

/**
 * Generate Message Authentication Code for given data
 *
 * https://tools.ietf.org/html/rfc4493
 *
 * @param data      Data to generate code for
 * @param len       Data length
 * @param key       Key to use for encryption
 * @param tag       Buffer to store result to (16 bytes tag)
 */
void AES128_CMAC(const uint8_t *data, size_t len, const uint8_t *key,
        uint8_t *tag);

#endif
