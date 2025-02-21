/**
 * @file    utils/string.h
 * @brief   Helper functions for working with strings
 */

#ifndef __UTILS_STRING_H
#define __UTILS_STRING_H

#include <types.h>

/**
 * Convert hex digit to decimal value
 *
 * @param c     Character to be converted
 * @return decimal value of hex or 0 if failed (can use isxdigit() to check)
 */
uint8_t hex2dec(char c);

/**
 * Convert decimal value to hex digit
 *
 * @param num   Number to be converted (0-15)
 * @return chracter respresenting hex value od '0' if not possible to convert
 */
char dec2hex(uint8_t num);

/**
 * Convert number to hex string
 *
 * Make sure the result will fit the buf - length must be
 * places + 1 (end of string character)
 *
 * @param value     Number to be converted
 * @param places    Amount of places to print (num=0x1234, places=2 => outputs 34)
 * @param buf       Buffer to store result to
 */
void num2hex(uint32_t value, uint8_t places, char *buf);

/**
 * Convert number to terminated string
 *
 * @param value     Number to be converted
 * @param buf       Buffer to store data to
 * @param len       Length of the buffer, buf won't be terminated if len is 0
 */
void num2str(uint32_t value, char *buf, size_t len);

#endif
