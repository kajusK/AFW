/**
 * @file    utils/physics.c
 * @brief   Physics calculations
 */

#include <math.h>
#include "physics.h"

int32_t pressureToAltM(uint32_t pressure_pa, uint32_t sea_level_pa)
{
    /*
     * Based on hypsometric formula, assuming 15 degrees at MSL
     *
     * TODO rewrite with lookup table? Takes 10k of flash because of math.h
     */
    return 44330.0 * (1.0 - pow((float)pressure_pa / sea_level_pa, 0.1902949571836346));
}
