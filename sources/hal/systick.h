/**
 * @file    hal/systick.h
 * @brief   Systick driver
 */

#ifndef __HAL_SYSTICK_H
#define __HAL_SYSTICK_H

#include <types.h>

typedef void (*systickd_cb_t)(void);

/**
 * Set callback which should be called upon systick interrupt
 *
 * Called in interrupt!
 *
 * @param cb        Callback to be called
 */
void Systickd_SetCallback(systickd_cb_t cb);

/**
 * Initialize systick timer
 *
 * @param frequency Frequency of systick interrupt
 */
void Systickd_Init(void);

#endif
