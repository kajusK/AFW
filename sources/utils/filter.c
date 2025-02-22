/**
 * @file    filters.c
 * @brief   Various digital filters
 */

#include <types.h>
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
    gain = (kalman->state_uncert * 10000) / (kalman->state_uncert + kalman->meas_uncert);
    new_state = kalman->state + (gain * (value - kalman->state) + 5000) / 10000;
    kalman->state_uncert = ((10000 - gain) * kalman->state_uncert + 5000) / 10000 +
                           (abs(kalman->state - new_state) * kalman->variance + 500) / 1000;
    kalman->state = new_state;
    return new_state;
}

void Filter_KalmanSimpleInit(filter_t *filter, int32_t state, int32_t meas_uncertainty,
    int32_t variance)
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
