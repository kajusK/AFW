/**
 * @file    utils/random.c
 * @brief   Random number generator
 */

#ifndef __UTILS_RANDOM_H
#define __UTILS_RANDOM_H

#include <types.h>

/** Get random number */
uint32_t Random_Get();

/**
 * Initialize the Random number generator with seed, the output sequence is
 * identical for the seed.
 */
void Random_Init(uint32_t seed);

#endif
