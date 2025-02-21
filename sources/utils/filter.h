/**
 * @file    utils/filter.h
 * @brief   Various digital filters
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
