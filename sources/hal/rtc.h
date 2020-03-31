/*
 * Copyright (C) 2019 Jakub Kaderka
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
 * @file    hal/rtc.h
 * @brief   Real Time clock
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_RTC_H
#define __HAL_RTC_H

#include <types.h>
#include <time.h>

/** Callback type for alarm events */
typedef void (*rtcd_alarm_cb_t)(void);

/**
 * Get RTC module current time
 *
 * @param tm    Structure to store data to
 */
extern void RTCd_GetTime(struct tm *tm);

/**
 * Set RTC module current time
 *
 * @param tm    Time structure
 */
extern void RTCd_SetTime(const struct tm *tm);

/**
 * Setup RTC wakeup timer to fire in given time
 *
 * Enables corresponding EXTI event for wakeup from low power modes.
 * NOT ALL DEVICES HAVE WAKEUP TIMER!
 *
 * @param time_s    Timeout in seconds to set timer to
 * @param persist   If true, keep the timer enabled, else disable interrupt
 *                  after first event
 */
extern void RTCd_SetWakeup(uint32_t time_s, bool persist);

/**
 * Set RTC alarm
 *
 * Enables corresponding EXTI lines for wakeup from low power modes.
 *
 * @param tm    Time at which alarm should trigger (only hour:min:sec used)
 * @param cb    Callback for alarm event (interrupt) or NULL if not needed
 */
extern void RTCd_SetAlarm(const struct tm *tm, rtcd_alarm_cb_t cb);

/**
 * Initialize RTC peripheral
 *
 * @param lse   Enable external 32768 Hz oscillator, use LSI if false
 * @return  True if initialized, false if already initialized (device rebooted)
 */
extern bool RTCd_Init(bool lse);

#endif

/** @} */
