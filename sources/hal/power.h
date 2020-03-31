/*
 * Copyright (C) 2020 Jakub Kaderka
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
 * @file    hal/power.h
 * @brief   System power control
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_POWER_H
#define __HAL_POWER_H

#include <types.h>

/** Reason of the system reset */
typedef enum {
    POWERD_RST_POR,         /**< Power on reset */
    POWERD_RST_NRST,        /**< NRST pin reset */
    POWERD_RST_WDG,         /**< Watchdog reset */
    POWERD_RST_SW,          /**< Reset requested by software */
    POWERD_RST_LOW_POWER,   /**< Low power management reset */
} powerd_rst_t;

/**
 * Configure debugging interface to work even in low-power modes
 *
 * It increases low-power mode consumption significantly, use only for
 * debugging purposes.
 *
 * Be aware that enabling debugging keeps system clock running even in stop
 * mode, therefore systick is still triggering interrupts if enabled, waking
 * up the MCU.
 */
extern void Powerd_EnableDebugging(void);

/**
 * Go to sleep mode
 *
 * Core clock halted, peripherals running, any interrupt processed by nvic
 * will wake the core. Turn unneeded peripherals off to save power consumption
 */
extern void Powerd_Sleep(void);

/**
 * Go to stop mode
 *
 * In stop mode, all peripherals except RTC are down, enabled interrupt on
 * EXTI line will wake device up. No need to turn off peripherals selectively.
 *
 * WDG is not affected and still running if enabled before
 */
extern void Powerd_Stop(void);

/**
 * Power off device (standby mode)
 *
 * Only backup registers are kept, HW reset, WKUP pin, watchdog or RTC alarm
 * can wake up the device (boots as if restarted). All IO pins are high
 * impedance during power off.
 */
extern void Powerd_Off(void);

/**
 * Reboot the MCU
 */
extern void Powerd_Reboot(void);

/**
 * Check if the MCU was in standby mode before powering on (clears the flag)
 */
extern bool Powerd_BootedFromStandby(void);

/**
 * Get reason of the system reset, flag is cleared after reading!
 *
 * @return type of the system reset
 */
extern powerd_rst_t Powerd_GetResetSource(void);

#endif

/** @} */
