/**
 * @file    hal/systick.c
 * @brief   Systick driver
 */

#include <types.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include "hal/systick.h"

static systickd_cb_t systickdi_cb;

void sys_tick_handler(void)
{
    if (systickdi_cb != NULL) {
        systickdi_cb();
    }
}

void Systickd_SetCallback(systickd_cb_t cb)
{
    systickdi_cb = cb;
}

void Systickd_Init(void)
{
    systick_clear();

    ASSERT(systick_set_frequency(1000, rcc_ahb_frequency));
    systick_counter_enable();
    systick_interrupt_enable();
}
