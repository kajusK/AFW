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
 * @file    hal/uart.c
 * @brief   UART driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>

#include "utils/assert.h"
#include "hal/uart.h"

static const uint32_t uartdi_regs[] = {
    USART1,
#ifdef USART2_BASE
    USART2,
#ifdef USART3_BASE
    USART3,
#ifdef USART4_BASE
    USART4,
#endif
#endif
#endif
};

static const uint32_t uartdi_rcc[] = {
    RCC_USART1,
#ifdef USART2_BASE
    RCC_USART2,
#ifdef USART3_BASE
    RCC_USART3,
#ifdef USART4_BASE
    RCC_USART4,
#endif
#endif
#endif
};

static const uint8_t uartdi_irq[] = {
    NVIC_USART1_IRQ,
#ifdef USART2_BASE
    NVIC_USART2_IRQ,
#ifdef USART3_BASE
    NVIC_USART3_4_IRQ,
#ifdef USART4_BASE
    NVIC_USART3_4_IRQ,
#endif
#endif
#endif
};

#define UARTD_INTERFACES (sizeof(uartdi_regs)/sizeof(uartdi_regs[0]))

static uartd_callback_t uartdi_rx_cb[UARTD_INTERFACES];

/**
 * Common handler for uart IRQ requests, send data to callback if defined
 *
 * @param device	Device ID (starts from 1)
 * @param uart		Address of first register of given uart
 */
static void UARTdi_IRQHandler(uint8_t device, uint32_t uart)
{
    uint8_t data;

    if ((USART_ISR(uart) & USART_FLAG_RXNE) == 0) {
        return;
    }

    data = usart_recv(uart);
    if (uartdi_rx_cb[device-1] != NULL) {
    	uartdi_rx_cb[device-1](data);
    }

    /*
     * There seems to be a buf, event if the flag is cleared by
     * reading the data, interrupt retriggers for some reason...
     */
    USART_ICR(uart) = (uint32_t) -1;
}

void usart1_isr(void)
{
	UARTdi_IRQHandler(1, USART1);
}

#ifdef USART2_BASE
void usart2_isr(void)
{
	UARTdi_IRQHandler(2, USART2);
}

#ifdef USART3_BASE
void usart3_4_isr(void)
{
    if ((USART_ISR(USART3) & USART_FLAG_RXNE) != 0) {
        UARTdi_IRQHandler(3, USART3);
    }
#ifdef USART4_BASE
    if ((USART_ISR(USART4) & USART_FLAG_RXNE) != 0) {
        UARTdi_IRQHandler(4, USART4);
    }
#endif
}
#endif
#endif

/**
 * Get UART device address from device id
 *
 * @param device	Device ID (starts from 1)
 * @return Address of the device's base register
 */
static uint32_t UARTdi_GetDevice(uint8_t device)
{
	ASSERT_NOT(device == 0 || device > UARTD_INTERFACES);
	return uartdi_regs[device - 1];
}

/**
 * Get UART device rcc register
 *
 * @param device	Device ID (starts from 1)
 * @return Address of the device's rcc register
 */
static enum rcc_periph_clken UARTdi_GetRcc(uint8_t device)
{
	ASSERT_NOT(device == 0 || device > UARTD_INTERFACES);
	return uartdi_rcc[device - 1];
}

static uint8_t UARTdi_GetIRQ(uint8_t device)
{
	ASSERT_NOT(device == 0 || device > UARTD_INTERFACES);
	return uartdi_irq[device - 1];
}

void UARTd_Write(uint8_t device, const uint8_t *buf, size_t len)
{
	uint32_t uart = UARTdi_GetDevice(device);

	while (len-- != 0) {
		usart_send_blocking(uart, *buf++);
	}
}

void UARTd_Puts(uint8_t device, const char *msg)
{
	uint32_t uart = UARTdi_GetDevice(device);

	while (*msg != '\0') {
		usart_send_blocking(uart, *msg++);
	}
}

void UARTd_Putc(uint8_t device, char c)
{
	uint32_t uart = UARTdi_GetDevice(device);
    usart_send_blocking(uart, c);
}

void UARTd_SetRxCallback(uint8_t device, uartd_callback_t callback)
{
	ASSERT_NOT(device == 0 || device > UARTD_INTERFACES);
	uartdi_rx_cb[device - 1] = callback;
}

void UARTd_Init(uint8_t device, uint32_t baudrate)
{
	enum rcc_periph_clken rcc = UARTdi_GetRcc(device);
	uint32_t uart = UARTdi_GetDevice(device);
	uint8_t irq = UARTdi_GetIRQ(device);

    rcc_periph_clock_enable(rcc);

    usart_set_baudrate(uart, baudrate);
    usart_set_databits(uart, 8);
    usart_set_stopbits(uart, USART_STOPBITS_1);
    usart_set_mode(uart, USART_MODE_TX_RX);
    usart_set_parity(uart, USART_PARITY_NONE);
    usart_set_flow_control(uart, USART_FLOWCONTROL_NONE);

    nvic_enable_irq(irq);
    usart_enable_rx_interrupt(uart);

    usart_enable(uart);
}

/** @} */
