/**
 * @file    hal/exti.c
 * @brief   Extended interrupts driver
 */

#include <types.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
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

void pvd_isr(void)
{
    EXTIdi_Int(EXTID_LINE_PVD);
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

void EXTId_SetMux(uint32_t port, uint8_t pad)
{
    ASSERT_NOT(pad > 15);
#ifdef RCC_AFIO
    /* AFIO clock required for muxing */
    rcc_periph_clock_enable(RCC_AFIO);
#endif
    exti_select_source(1 << pad, port);
}

void EXTId_SetEdge(uint8_t exti_num, extid_edge_t edge)
{
    uint32_t val = 1 << exti_num;
    ASSERT_NOT(exti_num > 31);

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
}

void EXTId_EnableEvent(uint8_t exti_num)
{
    uint32_t val = 1 << exti_num;
    ASSERT_NOT(exti_num > 31);

    exti_reset_request(val);
	EXTI_EMR |= val;
}

void EXTId_EnableInt(uint8_t exti_num)
{
    uint8_t irqn;
    uint32_t val = 1 << exti_num;
    ASSERT_NOT(exti_num > 31);

    exti_disable_request(val);
    switch (exti_num) {
        case 0:
        case 1:
            irqn = NVIC_EXTI0_1_IRQ;
            break;
        case 2:
        case 3:
            irqn = NVIC_EXTI2_3_IRQ;
            break;
        case EXTID_LINE_PVD:
            irqn = NVIC_PVD_IRQ;
            break;
        default:
            irqn = NVIC_EXTI4_15_IRQ;
            break;
    }

    exti_reset_request(val);
	EXTI_IMR |= val;
    nvic_enable_irq(irqn);
}

void EXTId_Disable(uint8_t exti_num)
{
    ASSERT_NOT(exti_num > 31);
    exti_disable_request(1 << exti_num);
}
