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
 * @file    hal/exti.c
 * @brief   Extended interrupts driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>

#include "utils/assert.h"
#include "hal/exti.h"

static void EXTIdi_Int(uint8_t exti_num);

extid_callback_t extidi_cb;

void exti0_1_isr(void)
{
    if (exti_get_flag_status(1 << 0)) {
        EXTIdi_Int(0);
    }
    if (exti_get_flag_status(1 << 1)) {
        EXTIdi_Int(1);
    }
}

void exti2_3_isr(void)
{
    if (exti_get_flag_status(1 << 2)) {
        EXTIdi_Int(2);
    }
    if (exti_get_flag_status(1 << 3)) {
        EXTIdi_Int(3);
    }
}

void exti4_15_isr(void)
{
    for (uint8_t i = 4; i <= 15; i++) {
        if (exti_get_flag_status(1 << i)) {
            EXTIdi_Int(i);
        }
    }
}

/**
 * Process interrupt request
 *
 * @param exti_num  Number of the EXTI line from where the interrupt arrived
 */
static void EXTIdi_Int(uint8_t exti_num)
{
    exti_reset_request(1 << exti_num);
    if (extidi_cb != NULL) {
        extidi_cb(exti_num);
    }
}

void EXTId_SetCallback(extid_callback_t cb)
{
    extidi_cb = cb;
}

void EXTId_SetMux(uint8_t exti_num, uint32_t port)
{
    ASSERT_NOT(exti_num > 15);
#ifdef RCC_AFIO
    /* AFIO clock required for muxing */
    rcc_periph_clock_enable(RCC_AFIO);
#endif
    exti_select_source(1 << exti_num, port);
}

void EXTId_Enable(uint8_t exti_num, extid_edge_t edge)
{
    uint8_t irqn;
    uint32_t val = 1 << exti_num;

    ASSERT_NOT(exti_num > 31);

    exti_disable_request(val);
    switch (edge) {
       case EXTID_RISING:
           EXTI_RTSR |= val;
           EXTI_FTSR &= ~val;
           break;
       case EXTID_FALLING:
           EXTI_RTSR &= ~val;
           EXTI_FTSR |= val;
           break;
       case EXTID_BOTH:
           EXTI_RTSR |= val;
           EXTI_FTSR |= val;
           break;
       default:
           break;
   }

    switch (exti_num) {
        case 0:
        case 1:
            irqn = NVIC_EXTI0_1_IRQ;
            break;
        case 2:
        case 3:
            irqn = NVIC_EXTI2_3_IRQ;
            break;
        default:
            irqn = NVIC_EXTI4_15_IRQ;
            break;
    }

    exti_reset_request(val);
    exti_enable_request(val);
    nvic_enable_irq(irqn);
}

void EXTId_Disable(uint8_t exti_num)
{
    ASSERT_NOT(exti_num > 31);
    EXTI_IMR &= ~(1 << exti_num);
}

/** @} */
