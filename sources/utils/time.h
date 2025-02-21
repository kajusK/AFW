/**
 * @file    utils/time.h
 * @brief   Usual time measurement utilities - millis, etc.
 */

#ifndef __UTILS_TIME_H
#define __UTILS_TIME_H

#include <types.h>

/**
 * Get time since initialization in milliseconds
 *
 * @return time since initialization
 */
extern uint32_t millis(void);

/**
 * Wait for given amount of time (in ms)
 *
 * Precission is up to 1 ms
 *
 * @param ms    Amount of milliseconds to wait for
 */
extern void delay_ms(uint32_t ms);

/**
 * Initialize timer module (start systick, etc)
 */
extern void Time_Init(void);

#endif
