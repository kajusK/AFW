/**
 * @file    utils/random.c
 * @brief   Random number generator
 */

#include <types.h>
#include "random.h"

static uint32_t state;

uint32_t Random_Get(void)
{
    /* Simple Xorshift random number generator */
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

void Random_Init(uint32_t seed)
{
    state = seed;
}
