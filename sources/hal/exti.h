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
 * @file    hal/exti.c
 * @brief   Extended interrupts driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_EXTI_H
#define __HAL_EXTI_H

#include <types.h>

/**
 * Callback type for EXTI interrupts
 *
 * @param exti_num  Number of the EXTI channel that received interrupt
 */
typedef void (*extid_callback_t)(uint8_t exti_num);

/** Edge type to be sensitive to */
typedef enum {
    EXTID_RISING,
    EXTID_FALLING,
    EXTID_BOTH,
} extid_edge_t;

#define EXTID_LINE_RTC_ALARM 17
#define EXTID_LINE_USB 18
#define EXTID_LINE_TAMPER 19
//TODO available only on stm32f0x0xC devices */
#define EXTID_LINE_RTC_WAKEUP 20

/**
 * Set callback for the exti interrupts
 */
extern void EXTId_SetCallback(extid_callback_t cb);

/**
 * Set input multiplexer to connect a GPIO pin to a given EXTI line
 *
 * Only EXTI 0-15 can be configured
 *
 * @param exti_num  Number of the gpio pin and EXTI line to set mux for
 * @param gpio_port Port address to be assigned to given EXTI line (GPIOA,...)
 */
extern void EXTId_SetMux(uint8_t exti_num, uint32_t port);

/**
 * Enable EXTI interrupt
 *
 * Only EXTI 0-15 will trigger interrupt callback!
 *
 * @param exti_num  EXTI id to enable interrupt for
 * @param edge  Edge type to trigger the interrupt
 */
extern void EXTId_Enable(uint8_t exti_num, extid_edge_t edge);

/**
 * Disable EXTI interrupt
 *
 * @param exti_num  EXTI id to disable interrupt for
 */
extern void EXTId_Disable(uint8_t exti_num);

#endif

/** @} */
