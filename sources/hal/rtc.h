/**
 * @file    hal/rtc.h
 * @brief   Real Time clock
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
void RTCd_GetTime(struct tm *tm);

/**
 * Set RTC module current time
 *
 * @param tm    Time structure
 */
void RTCd_SetTime(const struct tm *tm);

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
void RTCd_SetWakeup(uint32_t time_s, bool persist);

/**
 * Set RTC alarm
 *
 * Enables corresponding EXTI lines for wakeup from low power modes.
 *
 * @param tm    Time at which alarm should trigger (only hour:min:sec used)
 * @param cb    Callback for alarm event (interrupt) or NULL if not needed
 */
void RTCd_SetAlarm(const struct tm *tm, rtcd_alarm_cb_t cb);

/**
 * Set RTC alarm after given time - emulates WakeUp feature not present on all MCUs
 *
 * @param seconds Amount of seconds since now to raise alarm, up to 24 hours
 * @param cb    Callback for alarm event (interrupt) or NULL if not needed
 */
void RTCd_SetAlarmInSeconds(uint32_t seconds, rtcd_alarm_cb_t cb);

/**
 * Initialize RTC peripheral
 *
 * @param lse   Enable external 32768 Hz oscillator, use LSI if false
 * @return  True if initialized, false if already initialized (device rebooted)
 */
bool RTCd_Init(bool lse);

#endif
