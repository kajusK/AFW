/**
 * @file    hal/power.c
 * @brief   System power control
 */

#include <types.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/stm32/rtc.h>

#include "hal/rtc.h"
#include "hal/power.h"

/**
 * Execute wait for interrupt sleep instruction
 */
static inline __attribute__((always_inline)) void __WFI(void)
{
    __asm volatile("wfi");
}

/**
 * Execute wait for event sleep instruction
 */
static inline __attribute__((always_inline)) void __WFE(void)
{
    __asm volatile("wfe");
}

/**
 * Execute data synchronization barrier instruction
 */
static inline __attribute__((always_inline)) void __DSB(void)
{
    __asm volatile("dsb");
}

static void Powerdi_Sleep(void (*instruction)(void))
{
    SCB_SCR &= ~(SCB_SCR_SLEEPDEEP | SCB_SCR_SEVONPEND | SCB_SCR_SLEEPONEXIT);
    PWR_CR |= PWR_CR_CWUF;
    instruction();

    /* RTC should be resynchronized if needed */
    if (RCC_BDCR & RCC_BDCR_RTCEN) {
        rtc_wait_for_synchro();
    }
}

static void Powerdi_Stop(void (*instruction)(void))
{
    SCB_SCR |= SCB_SCR_SLEEPDEEP;
    /* Set stop mode, disable power regulator */
    PWR_CR &= ~PWR_CR_PDDS;
    PWR_CR |= PWR_CR_LPDS | PWR_CR_CWUF;
    /* sleep */
    instruction();

    // TODO runs from HSI after wake up, return to previously used oscillator
    /* RTC should be resynchronized if needed */
    if (RCC_BDCR & RCC_BDCR_RTCEN) {
        rtc_wait_for_synchro();
    }
}

void Powerd_EnableDebugging(void)
{
    DBGMCU_CR |= DBGMCU_CR_SLEEP | DBGMCU_CR_STOP | DBGMCU_CR_STANDBY;
}

void Powerd_SleepEvent(void)
{
    Powerdi_Sleep(__WFE);
}

void Powerd_Sleep(void)
{
    Powerdi_Sleep(__WFI);
}

void Powerd_StopEvent(void)
{
    Powerdi_Stop(__WFE);
}

void Powerd_Stop(void)
{
    Powerdi_Stop(__WFI);
}

void Powerd_Off(void)
{
    SCB_SCR |= SCB_SCR_SLEEPDEEP;
    /* Set standby mode */
    PWR_CR |= PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_CWUF;
    __WFI();
}

void Powerd_Reboot(void)
{
    SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
    __DSB();
    while (1) {
        ;
    }
}

void Powerd_SetPVDThreshold(uint8_t threshold)
{
    ASSERT_NOT(threshold > 0x07);
    pwr_enable_power_voltage_detect((uint8_t)threshold);
}

bool Powerd_BootedFromStandby(void)
{
    if (PWR_CSR & PWR_CSR_SBF) {
        PWR_CR |= PWR_CR_CSBF;
        return true;
    }
    return false;
}

powerd_rst_t Powerd_GetResetSource(void)
{
    powerd_rst_t rst = POWERD_RST_POR;

    if (RCC_CSR & RCC_CSR_LPWRRSTF) {
        rst = POWERD_RST_LOW_POWER;
    } else if (RCC_CSR & (RCC_CSR_WWDGRSTF | RCC_CSR_IWDGRSTF)) {
        rst = POWERD_RST_WDG;
    } else if (RCC_CSR & RCC_CSR_SFTRSTF) {
        rst = POWERD_RST_SW;
    } else if (RCC_CSR & RCC_CSR_PORRSTF) {
        rst = POWERD_RST_POR;
    } else if (RCC_CSR & RCC_CSR_PINRSTF) {
        rst = POWERD_RST_NRST;
    }

    RCC_CSR |= RCC_CSR_RMVF;
    return rst;
}
