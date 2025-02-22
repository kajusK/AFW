/**
 * @file    crc.c
 * @brief   CRC calculations
 */

#ifndef __UTILS_CRC_H
#define __UTILS_CRC_H

#include <types.h>

/** Initial value for CRC16 calculation */
#define CRC16_INITIAL_VALUE 0xFFFFU
/** Initial value for CRC8 calculation */
#define CRC8_INITIAL_VALUE  0xFFU

/**
 * Calculate CRC from initial value and single byte
 *
 * @param [in] buf  Byte to calculate crc for
 * @param [in] crc  Initial CRC value
 *
 * @return CRC 16 (polynomial 0x1021 16)
 */
uint16_t CRC16_Add(uint8_t buf, uint16_t crc);

/**
 * Calculate CRC for buffer
 *
 * @param [in] buf  Data to calculate crc for
 * @param [in] len  Length of data buffer
 *
 * @return CRC 16 (polynomial 0x1021 16)
 */
uint16_t CRC16(const uint8_t *buf, uint32_t len);

/**
 * Calculate CRC from initial value and single byte
 *
 * @param [in] buf  Byte to calculate crc for
 * @param [in] crc  Initial CRC value
 *
 * @return CRC 8 (polynomial 0x31)
 */
uint16_t CRC8_Add(uint8_t buf, uint8_t crc);

/**
 * Calculate CRC for buffer
 *
 * @param [in] buf  Data to calculate crc for
 * @param [in] len  Length of data buffer
 *
 * @return CRC 8 (polynomial 0x31)
 */
uint8_t CRC8(const uint8_t *buf, uint32_t len);

#endif
