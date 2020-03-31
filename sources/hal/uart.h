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
 * @file    hal/uart.h
 * @brief   UART driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_UART_H
#define __HAL_UART_H

#include <types.h>

/** byte received callback */
typedef void (* uartd_callback_t)(uint8_t byte);

/**
 * Send data over uart in blocking mode
 *
 * @param device	Device ID (stars from 1)
 * @param [in] buf	Data to be send
 * @param len		Length of the data buffer
 */
extern void UARTd_Write(uint8_t device, const uint8_t *buf, size_t len);

/**
 * Send string over uart in blocking mode
 *
 * @param device    Device ID (starts from 1)
 * @param [in] msg  Null terminated string
 */
extern void UARTd_Puts(uint8_t device, const char *msg);

/**
 * Send single character to uart
 *
 * @param device    Device ID (starts from 1)
 * @param c         Character to be printed
 */
extern void UARTd_Putc(uint8_t device, char c);

/**
 * Set callback for byte received, callback is called from interrupt!
 *
 * @param device	Device ID (stars from 1)
 * @param callback	Callback to be called upon byte receiving
 */
extern void UARTd_SetRxCallback(uint8_t device, uartd_callback_t callback);

/**
 * Initialize UART device
 *
 * @param device	Device ID (stars from 1)
 * @param baudrate	Required uart baudrate
 */
extern void UARTd_Init(uint8_t device, uint32_t baudrate);

#endif

/** @} */
