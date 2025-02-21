/**
 * @file    hal/wdg.c
 * @brief   Watchdog driver
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/iwdg.h>

#include "hal/wdg.h"

void Wdgd_Init(uint32_t period_ms)
{
    rcc_periph_clock_enable(RCC_WWDG);
    iwdg_set_period_ms(period_ms);
    iwdg_start();
}

void Wdgd_Clear(void)
{
    iwdg_reset();
}
