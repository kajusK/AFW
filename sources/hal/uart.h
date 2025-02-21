/**
 * @file    hal/uart.h
 * @brief   UART driver
 */

#ifndef __HAL_UART_H
#define __HAL_UART_H

#include <types.h>

/** byte received callback */
typedef void (* uartd_callback_t)(uint8_t byte);

/**
 * Send data over uart in blocking mode
 *
 * @param device    Device ID (stars from 1)
 * @param [in] buf    Data to be send
 * @param len        Length of the data buffer
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
 * @param device    Device ID (stars from 1)
 * @param callback    Callback to be called upon byte receiving
 */
extern void UARTd_SetRxCallback(uint8_t device, uartd_callback_t callback);

/**
 * Change peripheral baudrate
 *
 * @param device    Device ID (stars from 1)
 * @param baudrate    Required uart baudrate
 */
extern void UARTd_SetBaudrate(uint8_t device, uint32_t baudrate);

/**
 * Initialize UART device
 *
 * @param device    Device ID (stars from 1)
 * @param baudrate    Required uart baudrate
 */
extern void UARTd_Init(uint8_t device, uint32_t baudrate);

#endif
