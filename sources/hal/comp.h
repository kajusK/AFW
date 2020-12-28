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
 * @file    hal/comp.h
 * @brief   Analog comparator driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_COMP_H
#define __HAL_COMP_H

#include <types.h>
#include <libopencm3/stm32/comparator.h>

/** Comparator speed (higher speed = higher consumption) */
typedef enum {
    COMP_SPEED_HIGH = COMP_CSR_SPEED_HIGH,
    COMP_SPEED_MEDIUM = COMP_CSR_SPEED_MED,
    COMP_SPEED_LOW = COMP_CSR_SPEED_LOW,
    COMP_SPEED_VERY_LOW = COMP_CSR_SPEED_VERYLOW,
} comp_speed_t;

/** Comparator hystersis */
typedef enum {
    COMP_HYST_HIGH = COMP_CSR_HYST_HIGH,
    COMP_HYST_MEDIUM = COMP_CSR_HYST_MED,
    COMP_HYST_LOW = COMP_CSR_HYST_LOW,
    COMP_HYST_NO = COMP_CSR_HYST_NO,
} comp_hyst_t;

/** Comparator negative input selection */
typedef enum {
    COMP_IN_VREF_1_4 = COMP_CSR_INSEL_1_4_VREFINT,
    COMP_IN_VREF_2_4 = COMP_CSR_INSEL_2_4_VREFINT,
    COMP_IN_VREF_3_4 = COMP_CSR_INSEL_3_4_VREFINT,
    COMP_IN_VREF = COMP_CSR_INSEL_VREFINT,
    COMP_IN_INM4 = COMP_CSR_INSEL_INM4,
    COMP_IN_INM5 = COMP_CSR_INSEL_INM5,
    COMP_IN_INM6 = COMP_CSR_INSEL_INM6
} comp_in_neg_t;

/** Comparator output selection (use exti for interrupts) */
typedef enum {
    COMP_OUT_NONE = COMP_CSR_OUTSEL_NONE,
    COMP_OUT_TIM1_IC1 = COMP_CSR_OUTSEL_TIM1_IC1,
    COMP_OUT_TIM2_IC4 = COMP_CSR_OUTSEL_TIM2_IC4,
    COMP_OUT_TIM3_IC1 = COMP_CSR_OUTSEL_TIM3_IC1,
} comp_out_t;

/**
 * Enable comparator
 *
 * @param channel   Comparator channel
 */
extern void Compd_Enable(uint8_t channel);

/**
 * Disable comparator
 *
 * @param channel   Comparator channel
 */
extern void Compd_Disable(uint8_t channel);

/**
 * Initialize comparator
 *
 * @param channel   Comparator channel
 * @param speed     Comparator speed (higher = higher consumption)
 * @param hyst      Comparator hystersis
 * @param input     Comparator negative input selection
 * @param output    Comparator output selection (exti trigger always connected)
 * @param invert    Invert comparator output
 */
extern void Compd_Init(uint8_t channel, comp_speed_t speed, comp_hyst_t hyst,
        comp_in_neg_t input, comp_out_t output, bool invert);

#endif

/** @} */
