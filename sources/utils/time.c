/**
 * @file    utils/time.c
 * @brief   Usuall time measurement utilities - millis, etc.
 */

#include "hal/systick.h"
#include "utils/time.h"

static volatile uint32_t timei_elapsed_ms;

static void Timei_Systick(void)
{
    timei_elapsed_ms++;
}

uint32_t millis(void)
{
    return timei_elapsed_ms;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();

    while ((millis() - start) < ms) {
        ;
    }
}

void Time_Init(void)
{
    Systickd_Init();
    Systickd_SetCallback(Timei_Systick);
}
