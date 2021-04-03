/*
 * Copyright (C) 2021 Jakub Kaderka
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
 * @file    filters.c
 * @brief   Various digital filters
 *
 * @addtogroup utils
 * @{
 */

#include <utils/assert.h>
#include "filter.h"

/**
 * Calculate a new state estimate using a simple 1D Kalman filter
 *
 * https://www.kalmanfilter.net/kalman1d.html
 *
 * @param filter        Filter descriptor
 * @param value         New measured system state
 *
 * @return New estimated system state
 */
static int32_t Filteri_KalmanSimple(filter_t *filter, int32_t value)
{
    int32_t gain;
    int32_t new_state;
    filter_kalman_simple_t *kalman = &filter->kalman_simple;

    /* Multiplied by 10000 to avoid using float, +5000 to round result correctly */
    gain = (kalman->state_uncert*10000)/(kalman->state_uncert + kalman->meas_uncert);
    new_state = kalman->state + (gain*(value - kalman->state)+5000)/10000;
    kalman->state_uncert = ((10000 - gain)*kalman->state_uncert + 5000)/10000 + (abs(kalman->state - new_state)*kalman->variance + 500)/1000;
    kalman->state = new_state;
    return new_state;
}

void Filter_KalmanSimpleInit(filter_t *filter, int32_t state,
        int32_t meas_uncertainty, int32_t variance)
{
    ASSERT_NOT(filter == NULL);
    filter->type = FILTER_KALMAN_SIMPLE;
    filter->kalman_simple.meas_uncert = meas_uncertainty;
    filter->kalman_simple.state_uncert = meas_uncertainty;
    filter->kalman_simple.state = state;
    filter->kalman_simple.variance = variance;
}

int32_t Filter(filter_t *filter, int32_t value)
{
    ASSERT_NOT(filter == NULL);

    switch (filter->type) {
        case FILTER_KALMAN_SIMPLE:
            return Filteri_KalmanSimple(filter, value);
            break;
        default:
            ASSERT(false);
            break;
    }
}

/** @} */
