/**
 * @file    hal/wdg.h
 * @brief   Watchdog driver
 */

#ifndef __HAL_WDG_H
#define __HAL_WDG_H

#include <types.h>

/**
 * Initialize watchdog
 *
 * @param period_ms     Watchdog timer period in millisecond
 */
extern void Wdgd_Init(uint32_t period_ms);

/**
 * Clear watchdog timer
 *
 * Once initialized, this function must be called periodically to avoid
 * WDG mcu reset
 */
extern void Wdgd_Clear(void);

#endif
