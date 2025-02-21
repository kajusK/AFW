/**
 * @file    utils/utils.c
 * @brief   Hard to place anywhere utilities
 */

#include <types.h>
#include "utils.h"

uint8_t count1s(uint32_t data)
{
#if !defined(UNIT_TEST) && __has_builtin(__builtin_popcount)
    return __builtin_popcount(data);
#else
    /*
    * Not the more common SWAR implementation - loop is faster in most cases and
    * we are not handling encryption to keep the O(1) time
    */
    uint8_t count = 0;
    while (data != 0) {
        data = data & (data - 1);
        count++;
    }
    return count;
#endif
}
