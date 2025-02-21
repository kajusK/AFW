/**
 * @file    hal/exti.c
 * @brief   Extended interrupts driver
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

#define EXTID_LINE_PVD 16
#define EXTID_LINE_RTC_ALARM 17
#define EXTID_LINE_USB 18
#define EXTID_LINE_TAMPER 19
//TODO available only on stm32f0x0xC devices */
#define EXTID_LINE_RTC_WAKEUP 20
#define EXTID_LINE_COMP1 21
#define EXTID_LINE_COMP2 22

/**
 * Set callback for the exti interrupts
 */
void EXTId_SetCallback(extid_callback_t cb);

/**
 * Set input multiplexer to connect a GPIO pin to a EXTI line
 *
 * @param port      GPIO port the pin is member of (GPIOA,...)
 * @param pad       GPIO pad to connect to EXTI line
 */
void EXTId_SetMux(uint32_t port, uint8_t pad);

/**
 * Set edge to trigger event/interrupt on
 *
 * @param exti_num  EXTI id to set edge for
 * @param edge      Edge polarity
 */
void EXTId_SetEdge(uint8_t exti_num, extid_edge_t edge);

/**
 * Enable EXTI event
 *
 * @param exti_num  EXTI id to enable event for
 */
void EXTId_EnableEvent(uint8_t exti_num);

/**
 * Enable EXTI interrupt
 *
 * @param exti_num  EXTI id to enable interrupt for
 */
void EXTId_EnableInt(uint8_t exti_num);

/**
 * Disable EXTI interrupt/event
 *
 * @param exti_num  EXTI id to disable interrupt for
 */
void EXTId_Disable(uint8_t exti_num);

#endif
