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
 * @file    hal/rtc.c
 * @brief   Real Time clock
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>

#include "hal/exti.h"
#include "hal/rtc.h"

/** Callback for rtc alarm event */
static rtcd_alarm_cb_t rtcdi_alarm_cb;

/** Wake up timer persistence - keep it enabled if true */
static bool rtcdi_wut_persist = false;

/**
 * RTC interrupt request processing
 */
void rtc_isr(void)
{
    /* Wake up timer */
    if (RTC_ISR & RTC_ISR_WUTF) {
        rtc_unlock();
        if (!rtcdi_wut_persist) {
            RTC_CR &= ~(RTC_CR_WUTIE | RTC_CR_WUTE);
        }
        RTC_ISR &= ~RTC_ISR_WUTF;
        rtc_lock();
        exti_reset_request(1 << EXTID_LINE_RTC_WAKEUP);
    }
    /* Alarm A */
    if (RTC_ISR & RTC_ISR_ALRAF) {
        rtc_unlock();
        RTC_CR &= ~(RTC_CR_ALRAIE | RTC_CR_ALRAE);
        RTC_ISR &= ~RTC_ISR_ALRAF;
        rtc_lock();
        exti_reset_request(1 << EXTID_LINE_RTC_ALARM);
        if (rtcdi_alarm_cb != NULL) {
            rtcdi_alarm_cb();
        }
    }
}

/**
 * Enter config mode of the RTC peripheral
 */
static void RTCdi_EnterConfig(void)
{
    rtc_unlock();
    RTC_ISR |= RTC_ISR_INIT;
    while (!(RTC_ISR & RTC_ISR_INITF)) {
        ;
    }
}

/**
 * Leave config mode of the RTC peripheral
 */
static void RTCdi_LeaveConfig(void)
{
    RTC_ISR &= ~(RTC_ISR_INIT);
    rtc_lock();
}

void RTCd_GetTime(struct tm *tm)
{
    uint32_t tr;
    uint32_t dr;

    /* reading TR locks shadow registers, reading DR unlocks them again */
    tr = RTC_TR;
    dr = RTC_DR;

    tm->tm_sec = (tr >> RTC_TR_ST_SHIFT) & RTC_TR_ST_MASK;
    tm->tm_sec = tm->tm_sec*10 + ((tr >> RTC_TR_SU_SHIFT) & RTC_TR_SU_MASK);
    tm->tm_min = (tr >> RTC_TR_MNT_SHIFT) & RTC_TR_MNT_MASK;
    tm->tm_min = tm->tm_min*10 + ((tr >> RTC_TR_MNU_SHIFT) & RTC_TR_MNU_MASK);
    tm->tm_hour = (tr >> RTC_TR_HT_SHIFT) & RTC_TR_HT_MASK;
    tm->tm_hour = tm->tm_hour*10 + ((tr >> RTC_TR_HU_SHIFT) & RTC_TR_HU_MASK);
    if (tr & RTC_TR_PM) {
        tm->tm_hour += 12;
    }

    tm->tm_mday = (dr >> RTC_DR_DT_SHIFT) & RTC_DR_DT_MASK;
    tm->tm_mday = tm->tm_mday*10 + ((dr >> RTC_DR_DU_SHIFT) & RTC_DR_DU_MASK);
    tm->tm_mon = (dr >> RTC_DR_MT_SHIFT) & RTC_DR_MT_MASK;
    tm->tm_mon = tm->tm_mon*10 + ((dr >> RTC_DR_MU_SHIFT) & RTC_DR_MU_MASK);
    tm->tm_mon -= 1;
    tm->tm_year = (dr >> RTC_DR_YT_SHIFT) & RTC_DR_YT_MASK;
    tm->tm_year = tm->tm_year*10 + ((dr >> RTC_DR_YU_SHIFT) & RTC_DR_YU_MASK);
    tm->tm_year += 100;
    tm->tm_wday = (dr >> RTC_DR_WDU_SHIFT) & RTC_DR_WDU_MASK;
    tm->tm_wday =  (tm->tm_wday + 1) % 7;
}

void RTCd_SetTime(const struct tm *tm)
{
    uint32_t dr = 0;
    uint32_t tr = 0;

    tr |= (tm->tm_sec % 10) << RTC_TR_SU_SHIFT;
    tr |= (tm->tm_sec / 10) << RTC_TR_ST_SHIFT;
    tr |= (tm->tm_min % 10) << RTC_TR_MNU_SHIFT;
    tr |= (tm->tm_min / 10) << RTC_TR_MNT_SHIFT;
    tr |= (tm->tm_hour % 10) << RTC_TR_HU_SHIFT;
    tr |= (tm->tm_hour / 10) << RTC_TR_HT_SHIFT;

    dr |= (tm->tm_mday % 10) << RTC_DR_DU_SHIFT;
    dr |= (tm->tm_mday / 10) << RTC_DR_DT_SHIFT;
    dr |= ((tm->tm_mon + 1) % 10) << RTC_DR_MU_SHIFT;
    dr |= ((tm->tm_mon + 1) / 10) << RTC_DR_MT_SHIFT;
    dr |= ((tm->tm_year - 100) % 10) << RTC_DR_YU_SHIFT;
    dr |= ((tm->tm_year - 100) / 10) << RTC_DR_YT_SHIFT;

    if (tm->tm_wday == 0) {
        dr |= 7 << RTC_DR_WDU_SHIFT;
    } else {
        dr |= (tm->tm_wday - 1) << RTC_DR_WDU_SHIFT;
    }

    RTCdi_EnterConfig();
    RTC_DR = dr;
    RTC_TR = tr;
    RTCdi_LeaveConfig();
}

void RTCd_SetWakeup(uint32_t time_s, bool persist)
{
    rtcdi_wut_persist = persist;

    rtc_unlock();
    rtc_set_wakeup_time(time_s, RTC_CR_WUCLKSEL_SPRE);
    /* Enable interrupt */
    RTC_CR |= RTC_CR_WUTIE;
    rtc_lock();

    /* Enable EXTI event to wake up the core from stop mode */
    EXTId_Enable(EXTID_LINE_RTC_WAKEUP, EXTID_RISING);
}

void RTCd_SetAlarm(const struct tm *tm, rtcd_alarm_cb_t cb)
{
    uint32_t alrmar = 0;

    rtcdi_alarm_cb = cb;

    /* ignore date field */
    alrmar = RTC_ALRMXR_MSK4;
    alrmar |= (tm->tm_hour % 10) << RTC_ALRMXR_HU_SHIFT;
    alrmar |= (tm->tm_hour / 10) << RTC_ALRMXR_HT_SHIFT;
    alrmar |= (tm->tm_min % 10) << RTC_ALRMXR_MNU_SHIFT;
    alrmar |= (tm->tm_min / 10) << RTC_ALRMXR_MNT_SHIFT;
    alrmar |= (tm->tm_sec % 10) << RTC_ALRMXR_SU_SHIFT;
    alrmar |= (tm->tm_sec / 10) << RTC_ALRMXR_ST_SHIFT;

    rtc_unlock();
    RTC_CR &= ~RTC_CR_ALRAE;
    /* No subseconds comparsion */
    RTC_ALRMASSR = 0;
    RTC_ALRMAR = alrmar;
    RTC_CR |= RTC_CR_ALRAIE | RTC_CR_ALRAE;
    rtc_lock();
    EXTId_Enable(EXTID_LINE_RTC_ALARM, EXTID_RISING);
}

bool RTCd_Init(bool lse)
{
    rcc_periph_clock_enable(RCC_PWR);
    rcc_periph_clock_enable(RCC_RTC);
    pwr_disable_backup_domain_write_protect();

    if (lse) {
        /* 32 767 Hz external crystall */
        rcc_osc_on(RCC_LSE);
        rcc_wait_for_osc_ready(RCC_LSE);
        rcc_set_rtc_clock_source(RCC_LSE);
    } else {
        /* 40 kHz internall RC oscillator */
        rcc_osc_on(RCC_LSI);
        rcc_wait_for_osc_ready(RCC_LSI);
        rcc_set_rtc_clock_source(RCC_LSI);
    }

    /* Clean interrupts - e.g. woken from standby */
    rtc_unlock();
    RTC_CR &= ~(RTC_CR_WUTIE | RTC_CR_WUTE | RTC_CR_ALRAIE | RTC_CR_ALRAE);
    RTC_ISR &= ~(RTC_ISR_ALRAF | RTC_ISR_WUTF);
    rtc_lock();
    exti_reset_request(1 << EXTID_LINE_RTC_ALARM);
    exti_reset_request(1 << EXTID_LINE_RTC_WAKEUP);

    /* already configured, exit */
    if (RTC_ISR & RTC_ISR_INITS) {
        return false;
    }

    RCC_BDCR |= RCC_BDCR_RTCEN;

    RTCdi_EnterConfig();
    RTC_CR = 0; /* 24 hour format */
    RTC_ISR = 0;
    if (lse) {
        rtc_set_prescaler(255, 127);
    } else {
        rtc_set_prescaler(319, 124);
    }
    RTCdi_LeaveConfig();

    rtc_wait_for_synchro();
    nvic_enable_irq(NVIC_RTC_IRQ);
    return true;
}

/** @} */
