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
 * @file    utils/filter.h
 * @brief   Various digital filters
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_FILTER_H
#define __UTILS_FILTER_H

#include <types.h>

/** Filter types */
typedef enum {
    FILTER_KALMAN_SIMPLE,   /**< Simple 1D Kalman filter */
} filter_type_t;

/** Simple Kalman filter data */
typedef struct {
    int32_t state;          /**< Current calculated system state */
    int32_t state_uncert;   /**< Current state uncertainty */
    int32_t meas_uncert;    /**< Measurement uncertainty */
    int32_t variance;       /**< Speed of value change in 0.001 steps */
} filter_kalman_simple_t;

/** Filter descriptor */
typedef struct {
    filter_type_t type;     /**< Type of the filter */
    union {
        filter_kalman_simple_t kalman_simple;   /**< Simple 1D kalman filter */
    };
} filter_t;

/**
 * Initialize a simple Kalman filter
 *
 * (https://www.kalmanfilter.net/kalman1d.html)
 *
 * @param filter                Filter descriptor
 * @param state                 Initial system state
 * @param meas_uncertainty      Uncertainty of the measurement
 * @param variance              How fast is the state changing in 0.001 steps
 */
extern void Filter_KalmanSimpleInit(filter_t *filter, int32_t state,
        int32_t meas_uncertainty, int32_t variance);

/**
 * Apply selected filter to measured value
 *
 * @param filter            Filter descriptor
 * @param value             Measured value
 *
 * @return Filtered data
 */
extern int32_t Filter(filter_t *filter, int32_t value);

#endif

/** @} */
