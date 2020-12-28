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
 * @file    hal/timer.c
 * @brief   Timer driver
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>

#include "utils/assert.h"
#include "hal/timer.h"

static const enum tim_oc_id timerdi_oc_id[] = {TIM_OC1, TIM_OC2, TIM_OC3, TIM_OC4};
#define Timerdi_GetOcId(channel) timerdi_oc_id[(channel)]
static const enum tim_ic_id timerdi_ic_id[] = {TIM_IC1, TIM_IC2, TIM_IC3, TIM_IC4};
#define Timerdi_GetIcId(channel) timerdi_ic_id[(channel)]

static timerd_cb_t timerdi_cb[3];

/**
 * Get TIMER device address from device id
 *
 * @param device    Device ID (starts from 1)
 * @return Address of the device's base register
 */
static uint32_t Timerdi_GetDevice(uint8_t device)
{
    switch (device) {
        case 1:
            return TIM1;
            break;
        case 2:
            return TIM2;
            break;
        case 3:
            return TIM3;
            break;
        default:
            ASSERT(false);
            break;
    }
}

/**
 * Get Timer device RCC register
 *
 * @param device    Device ID (starts from 1)
 * @return Address of the device's RCC register
 */
static enum rcc_periph_clken Timerdi_GetRcc(uint8_t device)
{
    switch (device) {
        case 1:
            return RCC_TIM1;
            break;
        case 2:
            return RCC_TIM2;
            break;
        case 3:
            return RCC_TIM3;
            break;
        default:
            ASSERT(false);
            break;
    }
}

/**
 * Get Timer IRQ
 *
 * @param device    Device ID (starts from 1)
 * @return IRQ for the given device
 */
static uint8_t Timerdi_GetIRQ(uint8_t device)
{
    switch (device) {
        case 1:
            return NVIC_TIM1_CC_IRQ;
            break;
        case 2:
            return NVIC_TIM2_IRQ;
            break;
        case 3:
            return NVIC_TIM3_IRQ;
            break;
        default:
            ASSERT(false);
            break;
    }
}

/**
 * Interrupt handler common for all timers
 *
 * @param device        Device ID that caused the event
 */
static void Timerdi_IRQHandler(uint8_t device)
{
    uint32_t timer = Timerdi_GetDevice(device);
    uint32_t cc_mode = 0;
    uint32_t flag;
    timerd_event_t event = TIMER_EVENT_COMPARE;
    timerd_ch_t channel = TIMER_CH_1;


    /* Only one event per interrupt is processed */
    if ((TIM_DIER(timer) & TIM_DIER_UIE) && timer_get_flag(timer, TIM_SR_UIF)) {
        event = TIMER_EVENT_UPDATE;
        flag = TIM_SR_UIF;
    } else if ((TIM_DIER(timer) & TIM_DIER_CC1IE) && timer_get_flag(timer, TIM_SR_CC1IF)) {
        cc_mode = TIM_CCMR1(timer) & 0x03;
        flag = TIM_SR_CC1IF;
        channel = TIMER_CH_1;
    } else if ((TIM_DIER(timer) & TIM_DIER_CC2IE) && timer_get_flag(timer, TIM_SR_CC2IF)) {
        cc_mode = (TIM_CCMR1(timer) >> 8) & 0x03;
        flag = TIM_SR_CC2IF;
        channel = TIMER_CH_2;
    } else if ((TIM_DIER(timer) & TIM_DIER_CC3IE) && timer_get_flag(timer, TIM_SR_CC3IF)) {
        cc_mode = TIM_CCMR2(timer) & 0x03;
        flag = TIM_SR_CC3IF;
        channel = TIMER_CH_3;
    } else if ((TIM_DIER(timer) & TIM_DIER_CC4IE) & timer_get_flag(timer, TIM_SR_CC4IF)) {
        cc_mode = (TIM_CCMR2(timer) >> 8) & 0x03;
        flag = TIM_SR_CC3IF;
        channel = TIMER_CH_4;
    } else {
        ASSERT(false);
    }

    if (cc_mode != 0) {
        event = TIMER_EVENT_CAPTURE;
    }

    ASSERT(device > 0 && device - 1 < (int)(sizeof(timerdi_cb)/sizeof(timerdi_cb[0])));
    if (timerdi_cb[device - 1]) {
        timerdi_cb[device - 1](event, channel);
    }
    timer_clear_flag(timer, flag);
}

void tim1_brk_up_trg_com_isr(void)
{
    Timerdi_IRQHandler(1);
}

void tim1_cc_isr(void)
{
    Timerdi_IRQHandler(1);
}

void tim2_isr(void)
{
    Timerdi_IRQHandler(2);
}

void tim3_isr(void)
{
    Timerdi_IRQHandler(3);
}

void Timerd_EnableEvent(uint8_t device, timerd_event_t event,
        timerd_ch_t channel)
{
    uint32_t timer = Timerdi_GetDevice(device);

    switch (event) {
        case TIMER_EVENT_CAPTURE:
        case TIMER_EVENT_COMPARE:
            if (channel == TIMER_CH_1) {
                timer_clear_flag(timer, TIM_SR_CC1IF);
                timer_enable_irq(timer, TIM_DIER_CC1IE);
            } else if (channel == TIMER_CH_2) {
                timer_clear_flag(timer, TIM_SR_CC2IF);
                timer_enable_irq(timer, TIM_DIER_CC2IE);
            } else if (channel == TIMER_CH_3) {
                timer_clear_flag(timer, TIM_SR_CC3IF);
                timer_enable_irq(timer, TIM_DIER_CC3IE);
            } else if (channel == TIMER_CH_4) {
                timer_clear_flag(timer, TIM_SR_CC4IF);
                timer_enable_irq(timer, TIM_DIER_CC4IE);
            }
            break;
        case TIMER_EVENT_UPDATE:
            timer_clear_flag(timer, TIM_SR_UIF);
            timer_enable_irq(timer, TIM_DIER_UIE);
            break;
    }
}

void Timerd_DisableEvent(uint8_t device, timerd_event_t event,
        timerd_ch_t channel)
{
    uint32_t timer = Timerdi_GetDevice(device);

    switch (event) {
        case TIMER_EVENT_CAPTURE:
        case TIMER_EVENT_COMPARE:
            if (channel == TIMER_CH_1) {
                timer_disable_irq(timer, TIM_DIER_CC1IE);
            } else if (channel == TIMER_CH_2) {
                timer_disable_irq(timer, TIM_DIER_CC2IE);
            } else if (channel == TIMER_CH_3) {
                timer_disable_irq(timer, TIM_DIER_CC3IE);
            } else if (channel == TIMER_CH_4) {
                timer_disable_irq(timer, TIM_DIER_CC4IE);
            }
            break;
        case TIMER_EVENT_UPDATE:
            timer_disable_irq(timer, TIM_DIER_UIE);
            break;
    }
}

void Timerd_SetPrescaler(uint8_t device, uint32_t prescaler)
{
    timer_set_prescaler(Timerdi_GetDevice(device), prescaler);
}

uint32_t Timerd_GetFrequency(uint8_t device)
{
    uint32_t timer_clk;
    /* If APB prescaler is not 1, the timers clock is multiplied by 2 first */
    if ((RCC_CFGR & ~RCC_CFGR_PPRE) == RCC_CFGR_PPRE_NODIV) {
        timer_clk = rcc_apb1_frequency;
    } else {
        timer_clk = rcc_apb1_frequency*2;
    }

    return timer_clk / (TIM_PSC(Timerdi_GetDevice(device)) + 1);
}

void Timerd_SetClockFreq(uint8_t device, uint32_t freq_hz)
{
    uint32_t timer_clk;
    /* If APB prescaler is not 1, the timers clock is multiplied by 2 first */
    if ((RCC_CFGR & ~RCC_CFGR_PPRE) == RCC_CFGR_PPRE_NODIV) {
        timer_clk = rcc_apb1_frequency;
    } else {
        timer_clk = rcc_apb1_frequency*2;
    }

    ASSERT(freq_hz <= timer_clk);
    timer_set_prescaler(Timerdi_GetDevice(device),
            (timer_clk / freq_hz) - 1);
}

void Timerd_SetPeriod(uint8_t device, uint32_t period)
{
    timer_set_period(Timerdi_GetDevice(device), period);
}

void Timerd_SetCompare(uint8_t device, timerd_ch_t channel, uint32_t value)
{
    ASSERT(channel < 4);
    timer_set_oc_value(Timerdi_GetDevice(device), Timerdi_GetOcId(channel),
            value);
}

uint32_t Timerd_GetCapture(uint8_t device, timerd_ch_t channel)
{
    uint32_t timer = Timerdi_GetDevice(device);

    switch (channel) {
        case TIMER_CH_1:
            return TIM_CCR1(timer);
            break;
        case TIMER_CH_2:
            return TIM_CCR2(timer);
            break;
        case TIMER_CH_3:
            return TIM_CCR3(timer);
            break;
        case TIMER_CH_4:
            return TIM_CCR4(timer);
            break;
    }
    return 0;
}

void Timerd_Set(uint8_t device, uint32_t value)
{
    timer_set_counter(Timerdi_GetDevice(device), value);
}

uint32_t Timerd_Get(uint8_t device)
{
    return timer_get_counter(Timerdi_GetDevice(device));
}

void Timerd_Start(uint8_t device)
{
    timer_enable_counter(Timerdi_GetDevice(device));
}

void Timerd_Stop(uint8_t device)
{
    timer_disable_counter(Timerdi_GetDevice(device));
}

void Timerd_SetOneShot(uint8_t device, bool value)
{
    uint32_t timer = Timerdi_GetDevice(device);
    if (value) {
        timer_one_shot_mode(timer);
    } else {
        timer_continuous_mode(timer);
    }
}

void Timerd_DisableCompare(uint8_t device, timerd_ch_t channel)
{
    ASSERT(channel < 4);
    timer_disable_oc_output(Timerdi_GetDevice(device),
            Timerdi_GetOcId(channel));
}

void Timerd_EnableCompare(uint8_t device, timerd_ch_t channel, bool invert)
{
    ASSERT(channel < 4);
    uint32_t timer = Timerdi_GetDevice(device);
    enum tim_oc_id oc_id = Timerdi_GetOcId(channel);

    /* Sets the mode and configures CC2S as output */
    timer_set_oc_mode(timer, oc_id, TIM_OCM_ACTIVE);

    if (device == 1) {
        /* required on advanced timers to make output working */
        timer_enable_break_main_output(timer);
    }
    if (invert) {
        timer_set_oc_polarity_low(timer, oc_id);
    } else {
        timer_set_oc_polarity_high(timer, oc_id);
    }
    timer_enable_oc_output(timer, oc_id);
}

void Timerd_DisableCapture(uint8_t device, timerd_ch_t channel)
{
    timer_ic_disable(Timerdi_GetDevice(device), Timerdi_GetIcId(channel));
}

void Timerd_EnableCapture(uint8_t device, timerd_ch_t channel,
        timerd_edge_t edge)
{
    ASSERT(channel < 4);
    uint32_t timer = Timerdi_GetDevice(device);
    enum tim_ic_id ic_id = Timerdi_GetIcId(channel);
    uint32_t edge_val = 0;

    switch (edge) {
        case TIMER_EDGE_RISING:
            edge_val = 0;
            break;
        case TIMER_EDGE_FALLING:
            edge_val = 0x01;
            break;
        case TIMER_EDGE_BOTH:
            edge_val = 0x03;
            break;
    }
    TIM_CCER(timer) &= ~((0x03 << 1) << channel*4);
    TIM_CCER(timer) |= (edge_val << 1) << channel*4;

    /* Set channel to input */
    if (channel < 2) {
        TIM_CCMR1(timer) &= ~(0x03 << channel*8);
        TIM_CCMR1(timer) |= 0x01 << channel*8;
    } else {
        TIM_CCMR2(timer) &= ~(0x03 << (channel - 2)*8);
        TIM_CCMR2(timer) |= 0x01 << (channel - 2)*8;
    }

    timer_ic_set_prescaler(timer, ic_id, TIM_IC_PSC_OFF);
    timer_ic_set_filter(timer, ic_id, TIM_IC_CK_INT_N_8);
    timer_ic_enable(timer, ic_id);
}

void Timerd_EnablePWM(uint8_t device, timerd_ch_t channel, bool invert)
{
    ASSERT(channel < 4);
    uint32_t timer = Timerdi_GetDevice(device);
    enum tim_oc_id oc_id = Timerdi_GetOcId(channel);

    timer_enable_oc_preload(timer, oc_id);
    timer_set_oc_mode(timer, oc_id, TIM_OCM_PWM1);

    if (invert) {
        timer_set_oc_polarity_low(timer, oc_id);
    } else {
        timer_set_oc_polarity_high(timer, oc_id);
    }

    timer_enable_oc_output(timer, oc_id);
    if (device == 1) {
        /* required on advanced timers to make output working */
        timer_enable_break_main_output(timer);
    }
}

void Timerd_RegisterCb(uint8_t device, timerd_cb_t cb)
{
    /* indexing from zero */
    ASSERT(device > 0 && device - 1 < (int)(sizeof(timerdi_cb)/sizeof(timerdi_cb[0])));
    timerdi_cb[device - 1] = cb;
}

void Timerd_InitEncoder(uint8_t device, bool invert)
{
    uint32_t timer = Timerdi_GetDevice(device);
    ASSERT(device == 1 || device == 2 || device == 3);

    Timerd_Init(device);
    timer_slave_set_mode(timer, 0x3);
    timer_ic_set_input(timer, TIM_IC1, TIM_IC_IN_TI1);
    timer_ic_set_input(timer, TIM_IC2, TIM_IC_IN_TI2);
    timer_ic_set_filter(timer, TIM_IC1, TIM_IC_CK_INT_N_8);
    timer_ic_set_filter(timer, TIM_IC2, TIM_IC_CK_INT_N_8);

    TIM_CCER(timer) = 0;
    if (invert) {
        TIM_CCER(timer) |= TIM_CCER_CC1P | TIM_CCER_CC1P;
    }
    timer_enable_counter(timer);
}

void Timerd_Init(uint8_t device)
{
    uint32_t timer = Timerdi_GetDevice(device);

    rcc_periph_clock_enable(Timerdi_GetRcc(device));
    /* enable interrupts */
    if (device == 1) {
        nvic_enable_irq(NVIC_TIM1_BRK_UP_TRG_COM_IRQ);
        nvic_enable_irq(NVIC_TIM1_CC_IRQ);
    } else {
        nvic_enable_irq(Timerdi_GetIRQ(device));
    }

    /* No divider, edge alignment, up counting  */
    timer_set_mode(timer, TIM_CR1_CKD_CK_INT,
        TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_disable_preload(timer);
    timer_continuous_mode(timer);
    timer_set_prescaler(timer, 0);
    timer_set_period(timer, 0xffffffff);
}

/** @} */
